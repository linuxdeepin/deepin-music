#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Main test report generator for Qt autotest framework
"""

import json
import os
import re
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict

from .parsers.test_parser import TestOutputParser
from .parsers.coverage_parser import CoverageParser
from .generators.html_generator import HtmlReportGenerator
from .generators.csv_generator import CsvReportGenerator


class TestReportGenerator:
    """Main test report generator class"""
    
    def __init__(self, build_dir: str, report_dir: str, project_root: str, results_dir: str | None = None):
        self.build_dir = Path(build_dir)
        self.report_dir = Path(report_dir)
        self.project_root = Path(project_root)
        self.results_dir = Path(results_dir) if results_dir else self.project_root / "autotests" / ".results"
        self.test_data = {}
        
        # Initialize parsers and generators
        self.test_parser = TestOutputParser(self.report_dir, self.results_dir)
        self.coverage_parser = CoverageParser(self.build_dir, self.report_dir, self.project_root)
        self.html_generator = HtmlReportGenerator(self.build_dir, self.project_root)
        self.csv_generator = CsvReportGenerator(self.report_dir)
    
    def parse_test_output(self, test_passed: bool, test_duration: int) -> Dict:
        """Parse test output using TestOutputParser"""
        return self.test_parser.parse_test_output(test_passed, test_duration)
    
    def parse_coverage_data(self, coverage_success: bool, coverage_duration: int) -> Dict:
        """Parse coverage data using CoverageParser"""
        return self.coverage_parser.parse_coverage_data(coverage_success, coverage_duration)
    
    def collect_build_info(self) -> Dict:
        """Collect build information from CMake cache"""
        build_info = {
            "cmake_version": "Unknown",
            "compiler": "Unknown",
            "build_type": "Unknown",
            "build_time": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        }
        
        # Try to read from CMakeCache.txt
        cmake_cache = self.build_dir / "CMakeCache.txt"
        if cmake_cache.exists():
            try:
                with open(cmake_cache, 'r', encoding='utf-8') as f:
                    content = f.read()
                    
                cmake_version_match = re.search(r'CMAKE_VERSION:INTERNAL=(.+)', content)
                if cmake_version_match:
                    build_info["cmake_version"] = cmake_version_match.group(1)
                    
                compiler_match = re.search(r'CMAKE_CXX_COMPILER:FILEPATH=(.+)', content)
                if compiler_match:
                    build_info["compiler"] = os.path.basename(compiler_match.group(1))
                    
                build_type_match = re.search(r'CMAKE_BUILD_TYPE:STRING=(.+)', content)
                if build_type_match:
                    build_info["build_type"] = build_type_match.group(1)
                    
            except Exception as e:
                print(f"Error reading CMake cache: {e}")
                
        return build_info
    
    def generate_html_report(self, test_info: Dict, coverage_info: Dict, build_info: Dict) -> str:
        """Generate HTML report using HtmlReportGenerator"""
        return self.html_generator.generate_html_report(test_info, coverage_info, build_info)
    
    def generate_report(self, test_passed: bool, test_duration: int, 
                       coverage_success: bool, coverage_duration: int) -> bool:
        """Generate complete test report"""
        try:
            # Collect various data
            print("📊 Collecting test data...")
            test_info = self.test_parser.parse_all_results(test_passed, test_duration)
            
            print("📊 Collecting coverage data...")
            coverage_info = self.parse_coverage_data(coverage_success, coverage_duration)
            
            print("📊 Collecting build information...")
            build_info = self.collect_build_info()
            
            # Generate HTML report
            print("📊 Generating HTML report...")
            html_content = self.generate_html_report(test_info, coverage_info, build_info)
            
            # Write to file
            report_file = self.report_dir / "test_report.html"
            with open(report_file, 'w', encoding='utf-8') as f:
                f.write(html_content)
            
            # Generate JSON data file (for other tools)
            json_data = {
                "test_info": test_info,
                "coverage_info": coverage_info,
                "build_info": build_info,
                "generated_at": datetime.now().isoformat()
            }
            
            json_file = self.report_dir / "test_data.json"
            with open(json_file, 'w', encoding='utf-8') as f:
                json.dump(json_data, f, indent=2, ensure_ascii=False)

            # Generate CSV coverage report
            print("📊 Generating CSV coverage report...")
            self.csv_generator.generate_coverage_csv(coverage_info)

            print(f"✅ Report generated successfully: {report_file}")
            return True
            
        except Exception as e:
            print(f"❌ Error generating report: {e}")
            import traceback
            traceback.print_exc()
            return False


def _detect_test_state(report_dir: Path, results_dir: Path):
    """Auto-detect test pass/fail and duration from available data sources."""
    parser = TestOutputParser(report_dir, results_dir)
    test_info = parser.parse_all_results(True, 0)
    test_passed = test_info.get("passed", False) and test_info.get("failed_tests", 0) == 0
    test_duration = int(test_info.get("duration", 0))
    return test_passed, test_duration


def _detect_coverage_state(build_dir: Path):
    """Auto-detect coverage success from lcov output files."""
    coverage_info_file = build_dir / "coverage" / "filtered.info"
    return coverage_info_file.exists() and coverage_info_file.stat().st_size > 0


if __name__ == "__main__":
    import argparse

    cli = argparse.ArgumentParser(description="Generate Qt autotest report")
    cli.add_argument("--build-dir", required=True, help="Build directory path")
    cli.add_argument("--report-dir", required=True, help="Report output directory")
    cli.add_argument("--project-root", required=True, help="Project root path")
    cli.add_argument("--results-dir", default=None,
                     help="gtest XML results dir (default: <project-root>/autotests/.results)")
    cli.add_argument("--session", default=None,
                     help="session JSON path (default: <project-root>/autotests/.ut-session.json)")
    args = cli.parse_args()

    build_dir = Path(args.build_dir)
    report_dir = Path(args.report_dir)
    project_root = Path(args.project_root)
    results_dir = Path(args.results_dir) if args.results_dir else project_root / "autotests" / ".results"

    report_dir.mkdir(parents=True, exist_ok=True)

    test_passed, test_duration = _detect_test_state(report_dir, results_dir)
    coverage_success = _detect_coverage_state(build_dir)
    coverage_duration = 0

    generator = TestReportGenerator(str(build_dir), str(report_dir), str(project_root), str(results_dir))
    success = generator.generate_report(test_passed, test_duration, coverage_success, coverage_duration)
    sys.exit(0 if success else 1)
