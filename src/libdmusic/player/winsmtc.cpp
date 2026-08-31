// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "winsmtc.h"

#ifdef Q_OS_WIN

#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <roapi.h>
#include <windows.storage.streams.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Media;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Storage::Streams;

typedef ITypedEventHandler<SystemMediaTransportControls*,
                           SystemMediaTransportControlsButtonPressedEventArgs*> ButtonPressedHandler;

class ButtonDelegate : public ButtonPressedHandler
{
public:
    ButtonDelegate(WinSMTC *owner) : m_ref(1), m_owner(owner) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
    {
        if (!ppv) return E_POINTER;
        if (riid == __uuidof(IUnknown) || riid == __uuidof(ButtonPressedHandler)) {
            *ppv = static_cast<ButtonPressedHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_ref); }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG r = InterlockedDecrement(&m_ref);
        if (r == 0) delete this;
        return r;
    }

    HRESULT STDMETHODCALLTYPE Invoke(
        ISystemMediaTransportControls*,
        ISystemMediaTransportControlsButtonPressedEventArgs* args) override
    {
        if (args) {
            SystemMediaTransportControlsButton button;
            if (SUCCEEDED(args->get_Button(&button))) {
                switch (button) {
                case SystemMediaTransportControlsButton_Play:
                    Q_EMIT m_owner->playRequested();
                    break;
                case SystemMediaTransportControlsButton_Pause:
                    Q_EMIT m_owner->pauseRequested();
                    break;
                case SystemMediaTransportControlsButton_Stop:
                    Q_EMIT m_owner->stopRequested();
                    break;
                case SystemMediaTransportControlsButton_Next:
                    Q_EMIT m_owner->nextRequested();
                    break;
                case SystemMediaTransportControlsButton_Previous:
                    Q_EMIT m_owner->previousRequested();
                    break;
                default:
                    break;
                }
            }
        }
        return S_OK;
    }

private:
    LONG m_ref;
    WinSMTC *m_owner;
};

WinSMTC::WinSMTC(QObject *parent) : QObject(parent) {}

WinSMTC::~WinSMTC()
{
    shutdown();
}

bool WinSMTC::initialize(HWND hwnd)
{
    if (m_initialized) return true;
    if (!hwnd) {
        qWarning() << "WinSMTC: Invalid HWND";
        return false;
    }

    HRESULT hr = RoInitialize(RO_INIT_SINGLETHREADED);
    if (hr == RPC_E_CHANGED_MODE) {
        // COM already initialized in another mode, we don't own it
    } else if (SUCCEEDED(hr)) {
        m_roInitialized = true;
    } else {
        qWarning() << "WinSMTC: RoInitialize failed:" << QString::number(hr, 16);
        return false;
    }

    ComPtr<ISystemMediaTransportControlsInterop> interop;
    hr = RoGetActivationFactory(
        HStringReference(RuntimeClass_Windows_Media_SystemMediaTransportControls).Get(),
        IID_PPV_ARGS(&interop));
    if (FAILED(hr) || !interop) {
        qWarning() << "WinSMTC: Failed to get activation factory:" << QString::number(hr, 16);
        return false;
    }

    hr = interop->GetForWindow(hwnd, IID_PPV_ARGS(&m_smtc));
    if (FAILED(hr) || !m_smtc) {
        qWarning() << "WinSMTC: GetForWindow failed:" << QString::number(hr, 16);
        return false;
    }

    m_smtc->put_IsEnabled(true);

    m_smtc->get_DisplayUpdater(&m_updater);
    if (m_updater) {
        m_updater->put_Type(MediaPlaybackType_Music);
        m_updater->get_MusicProperties(&m_musicProps);
    } else {
        qWarning() << "WinSMTC: Failed to get display updater";
    }

    ButtonDelegate *handler = new ButtonDelegate(this);
    hr = m_smtc->add_ButtonPressed(handler, &m_buttonToken);
    handler->Release();

    if (FAILED(hr)) {
        qWarning() << "WinSMTC: Failed to add ButtonPressed handler:" << QString::number(hr, 16);
        m_buttonToken.value = 0;
    }

    m_initialized = true;
    return true;
}

void WinSMTC::shutdown()
{
    // Keep shutdown idempotent while also cleaning up partial initialization.
    if (!m_initialized && !m_roInitialized
            && !m_smtc.Get() && !m_updater.Get() && !m_musicProps.Get()) {
        return;
    }

    if (m_smtc) {
        // Unregister the ButtonPressed handler first to avoid use-after-free
        // when the system dispatches a media key after this object is gone
        if (m_buttonToken.value != 0) {
            m_smtc->remove_ButtonPressed(m_buttonToken);
            m_buttonToken.value = 0;
        }
        m_smtc->put_IsEnabled(false);
        m_smtc.Reset();
    }
    m_updater.Reset();
    m_musicProps.Reset();
    if (m_roInitialized) {
        RoUninitialize();
        m_roInitialized = false;
    }
    m_initialized = false;
}

void WinSMTC::updateMetadata(const QString &title, const QString &artist,
                            const QString &album, qint64 durationMs,
                            const QString &coverArtPath)
{
    if (!m_initialized || !m_updater || !m_musicProps) {
        qWarning() << "WinSMTC: updateMetadata called but not initialized";
        return;
    }

    m_updater->ClearAll();
    m_updater->put_Type(MediaPlaybackType_Music);
    m_updater->get_MusicProperties(&m_musicProps);

    m_musicProps->put_Title(HStringReference(title.toStdWString().c_str()).Get());
    m_musicProps->put_Artist(HStringReference(artist.toStdWString().c_str()).Get());

    ComPtr<IMusicDisplayProperties2> musicProps2;
    if (SUCCEEDED(m_musicProps.As(&musicProps2)) && musicProps2) {
        musicProps2->put_AlbumTitle(HStringReference(album.toStdWString().c_str()).Get());
    }

    // Set thumbnail from cover art file using in-memory stream
    if (!coverArtPath.isEmpty()) {
        QFileInfo fileInfo(coverArtPath);
        if (fileInfo.exists()) {
            QFile file(coverArtPath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray imageData = file.readAll();
                file.close();

                ComPtr<IInspectable> memStreamIns;
                HRESULT hrCreate = RoActivateInstance(
                    HStringReference(RuntimeClass_Windows_Storage_Streams_InMemoryRandomAccessStream).Get(),
                    &memStreamIns);
                
                if (SUCCEEDED(hrCreate) && memStreamIns) {
                    ComPtr<IRandomAccessStream> memStream;
                    HRESULT hrQI = memStreamIns.As(&memStream);
                    if (SUCCEEDED(hrQI) && memStream) {
                        ComPtr<IOutputStream> outputStream;
                        HRESULT hrOut = memStream->GetOutputStreamAt(0, &outputStream);
                        if (SUCCEEDED(hrOut) && outputStream) {
                            ComPtr<IDataWriterFactory> writerFactory;
                            HRESULT hrFactory = RoGetActivationFactory(
                                HStringReference(RuntimeClass_Windows_Storage_Streams_DataWriter).Get(),
                                IID_PPV_ARGS(&writerFactory));
                            if (SUCCEEDED(hrFactory) && writerFactory) {
                                ComPtr<IDataWriter> writer;
                                HRESULT hrWriter = writerFactory->CreateDataWriter(outputStream.Get(), &writer);
                                if (SUCCEEDED(hrWriter) && writer) {
                                    writer->WriteBytes(
                                        static_cast<unsigned int>(imageData.size()),
                                        reinterpret_cast<BYTE*>(imageData.data()));
                                    
                                    ComPtr<IAsyncOperation<unsigned int>> storeOp;
                                    if (SUCCEEDED(writer->StoreAsync(&storeOp)) && storeOp) {
                                        unsigned int written = 0;
                                        if (FAILED(storeOp->GetResults(&written))) {
                                            qWarning() << "WinSMTC: DataWriter StoreAsync failed";
                                        }
                                    }

                                    memStream->Seek(0);

                                    ComPtr<IRandomAccessStreamReferenceStatics> rasrStatics;
                                    HRESULT hrStatics = RoGetActivationFactory(
                                        HStringReference(RuntimeClass_Windows_Storage_Streams_RandomAccessStreamReference).Get(),
                                        IID_PPV_ARGS(&rasrStatics));
                                    if (SUCCEEDED(hrStatics) && rasrStatics) {
                                        ComPtr<IRandomAccessStreamReference> streamRef;
                                        HRESULT hrRef = rasrStatics->CreateFromStream(memStream.Get(), &streamRef);
                                        if (SUCCEEDED(hrRef) && streamRef) {
                                            m_updater->put_Thumbnail(streamRef.Get());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    HRESULT hr = m_updater->Update();
    if (FAILED(hr)) {
        qWarning() << "WinSMTC: DisplayUpdater::Update failed:" << QString::number(hr, 16);
    }
}

void WinSMTC::updatePlaybackStatus(int status)
{
    if (!m_initialized || !m_smtc) return;

    MediaPlaybackStatus winrtStatus;
    switch (status) {
    case 0: winrtStatus = MediaPlaybackStatus_Stopped; break;
    case 1: winrtStatus = MediaPlaybackStatus_Playing; break;
    case 2: winrtStatus = MediaPlaybackStatus_Paused; break;
    default: winrtStatus = MediaPlaybackStatus_Stopped; break;
    }

    m_smtc->put_PlaybackStatus(winrtStatus);
}

void WinSMTC::setControlsEnabled(bool play, bool pause, bool stop,
                                bool next, bool previous)
{
    if (!m_initialized || !m_smtc) return;

    m_smtc->put_IsPlayEnabled(play);
    m_smtc->put_IsPauseEnabled(pause);
    m_smtc->put_IsStopEnabled(stop);
    m_smtc->put_IsNextEnabled(next);
    m_smtc->put_IsPreviousEnabled(previous);
}

#endif // Q_OS_WIN
