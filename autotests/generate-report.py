#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Unit Test Report Generator (Wrapper for report_generator/main.py)
功能：调用 report_generator/main.py 生成单元测试报告
特性：
- 支持传入项目名
- 自动设置 Python 路径
- 调用专业的报告生成器
"""

import argparse
import sys
import os
from pathlib import Path

script_dir = Path(__file__).parent
resources_dir = script_dir.parent
sys.path.insert(0, str(resources_dir))

from report_generator.main import TestReportGenerator


def main():
    """Main function"""
    parser = argparse.ArgumentParser(description="Generate unit test report using report_generator")
    parser.add_argument("--build-dir", required=True, help="Build directory path")
    parser.add_argument("--report-dir", required=True, help="Report output directory path")
    parser.add_argument("--project-root", required=True, help="Project root directory path")
    parser.add_argument("--results-dir", default=None, help="gtest XML results dir (default: <project-root>/autotests/.results)")
    parser.add_argument("--test-passed", required=True, help="Whether tests passed (true/false)")
    parser.add_argument("--test-duration", type=int, required=True, help="Test duration in seconds")
    parser.add_argument("--coverage-success", required=True, help="Whether coverage generation succeeded (true/false)")
    parser.add_argument("--coverage-duration", type=int, required=True, help="Coverage generation duration in seconds")
    
    args = parser.parse_args()
    
    # Convert string parameters to boolean values
    test_passed = args.test_passed.lower() == 'true'
    coverage_success = args.coverage_success.lower() == 'true'
    
    # Create report generator
    generator = TestReportGenerator(args.build_dir, args.report_dir, args.project_root, args.results_dir)
    
    # Generate report
    success = generator.generate_report(
        test_passed, args.test_duration,
        coverage_success, args.coverage_duration
    )
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
