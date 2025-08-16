#!/usr/bin/env python3
"""
EnviroNet Analyzer - Test Results Analyzer
Analyzes test results and provides detailed insights into test performance and coverage.
"""

import json
import xml.etree.ElementTree as ET
import os
import sys
import argparse
from pathlib import Path
from datetime import datetime
import matplotlib.pyplot as plt
import pandas as pd

class TestAnalyzer:
    def __init__(self, test_results_dir="build", config_file="tests/test_configs.json"):
        self.test_results_dir = Path(test_results_dir)
        self.config_file = Path(config_file)
        self.test_configs = self.load_test_configs()
        self.results = {}
        
    def load_test_configs(self):
        """Load test configuration file."""
        try:
            with open(self.config_file, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            print(f"Warning: Test config file {self.config_file} not found")
            return {}
        except json.JSONDecodeError as e:
            print(f"Error parsing test config file: {e}")
            return {}
    
    def analyze_xml_results(self, xml_file):
        """Analyze XML test results from GoogleTest."""
        try:
            tree = ET.parse(xml_file)
            root = tree.getroot()
            
            # Extract test suite information
            test_suites = root.findall('.//testsuite')
            total_tests = 0
            total_failures = 0
            total_errors = 0
            total_time = 0.0
            
            suite_results = {}
            
            for suite in test_suites:
                suite_name = suite.get('name', 'Unknown')
                tests = int(suite.get('tests', 0))
                failures = int(suite.get('failures', 0))
                errors = int(suite.get('errors', 0))
                time = float(suite.get('time', 0.0))
                
                total_tests += tests
                total_failures += failures
                total_errors += errors
                total_time += time
                
                suite_results[suite_name] = {
                    'tests': tests,
                    'failures': failures,
                    'errors': errors,
                    'time': time,
                    'success_rate': ((tests - failures - errors) / tests * 100) if tests > 0 else 0
                }
            
            return {
                'total_tests': total_tests,
                'total_failures': total_failures,
                'total_errors': total_errors,
                'total_time': total_time,
                'success_rate': ((total_tests - total_failures - total_errors) / total_tests * 100) if total_tests > 0 else 0,
                'suite_results': suite_results
            }
            
        except ET.ParseError as e:
            print(f"Error parsing XML file {xml_file}: {e}")
            return None
        except Exception as e:
            print(f"Error analyzing XML file {xml_file}: {e}")
            return None
    
    def analyze_valgrind_log(self, valgrind_file):
        """Analyze Valgrind memory leak report."""
        try:
            with open(valgrind_file, 'r') as f:
                content = f.read()
            
            # Extract memory leak information
            definitely_lost = 0
            indirectly_lost = 0
            possibly_lost = 0
            still_reachable = 0
            
            # Parse Valgrind output
            for line in content.split('\n'):
                if 'definitely lost:' in line:
                    definitely_lost = int(line.split(':')[1].split()[0])
                elif 'indirectly lost:' in line:
                    indirectly_lost = int(line.split(':')[1].split()[0])
                elif 'possibly lost:' in line:
                    possibly_lost = int(line.split(':')[1].split()[0])
                elif 'still reachable:' in line:
                    still_reachable = int(line.split(':')[1].split()[0])
            
            return {
                'definitely_lost': definitely_lost,
                'indirectly_lost': indirectly_lost,
                'possibly_lost': possibly_lost,
                'still_reachable': still_reachable,
                'total_leaks': definitely_lost + indirectly_lost + possibly_lost
            }
            
        except FileNotFoundError:
            print(f"Valgrind log file {valgrind_file} not found")
            return None
        except Exception as e:
            print(f"Error analyzing Valgrind log {valgrind_file}: {e}")
            return None
    
    def collect_results(self):
        """Collect all test results from the build directory."""
        if not self.test_results_dir.exists():
            print(f"Test results directory {self.test_results_dir} not found")
            return
        
        # Find all XML result files
        xml_files = list(self.test_results_dir.glob("*.xml"))
        if not xml_files:
            print(f"No XML test result files found in {self.test_results_dir}")
            return
        
        print(f"Found {len(xml_files)} test result files")
        
        # Analyze each XML file
        for xml_file in xml_files:
            print(f"Analyzing {xml_file.name}...")
            result = self.analyze_xml_results(xml_file)
            if result:
                self.results[xml_file.stem] = result
        
        # Check for Valgrind log
        valgrind_file = self.test_results_dir / "valgrind.log"
        if valgrind_file.exists():
            print("Analyzing Valgrind memory report...")
            memory_result = self.analyze_valgrind_log(valgrind_file)
            if memory_result:
                self.results['memory'] = memory_result
    
    def generate_summary_report(self):
        """Generate a summary report of all test results."""
        if not self.results:
            print("No test results to analyze")
            return
        
        print("\n" + "="*60)
        print("ENVIRONET ANALYZER - TEST RESULTS SUMMARY")
        print("="*60)
        
        # Overall statistics
        total_tests = sum(r.get('total_tests', 0) for r in self.results.values() if isinstance(r, dict) and 'total_tests' in r)
        total_failures = sum(r.get('total_failures', 0) for r in self.results.values() if isinstance(r, dict) and 'total_failures' in r)
        total_errors = sum(r.get('total_errors', 0) for r in self.results.values() if isinstance(r, dict) and 'total_errors' in r)
        total_time = sum(r.get('total_time', 0.0) for r in self.results.values() if isinstance(r, dict) and 'total_time' in r)
        
        if total_tests > 0:
            overall_success_rate = ((total_tests - total_failures - total_errors) / total_tests) * 100
        else:
            overall_success_rate = 0
        
        print(f"\nOverall Results:")
        print(f"  Total Tests: {total_tests}")
        print(f"  Failures: {total_failures}")
        print(f"  Errors: {total_errors}")
        print(f"  Success Rate: {overall_success_rate:.1f}%")
        print(f"  Total Time: {total_time:.2f}s")
        
        # Memory analysis
        if 'memory' in self.results:
            mem = self.results['memory']
            print(f"\nMemory Analysis:")
            print(f"  Definitely Lost: {mem['definitely_lost']} bytes")
            print(f"  Indirectly Lost: {mem['indirectly_lost']} bytes")
            print(f"  Possibly Lost: {mem['possibly_lost']} bytes")
            print(f"  Still Reachable: {mem['still_reachable']} bytes")
            print(f"  Total Leaks: {mem['total_leaks']} bytes")
            
            if mem['total_leaks'] == 0:
                print("  ✅ No memory leaks detected!")
            else:
                print("  ⚠️  Memory leaks detected!")
        
        # Suite breakdown
        print(f"\nTest Suite Breakdown:")
        for name, result in self.results.items():
            if name == 'memory':
                continue
            
            if isinstance(result, dict) and 'suite_results' in result:
                for suite_name, suite_result in result['suite_results'].items():
                    status = "✅" if suite_result['success_rate'] == 100 else "⚠️" if suite_result['success_rate'] > 80 else "❌"
                    print(f"  {status} {suite_name}: {suite_result['success_rate']:.1f}% ({suite_result['tests']} tests, {suite_result['time']:.2f}s)")
    
    def generate_detailed_report(self, output_file=None):
        """Generate a detailed HTML report."""
        if not self.results:
            print("No test results to analyze")
            return
        
        html_content = self.generate_html_report()
        
        if output_file:
            with open(output_file, 'w') as f:
                f.write(html_content)
            print(f"Detailed report saved to {output_file}")
        else:
            # Save to default location
            report_file = self.test_results_dir / "test_report.html"
            with open(report_file, 'w') as f:
                f.write(html_content)
            print(f"Detailed report saved to {report_file}")
    
    def generate_html_report(self):
        """Generate HTML content for the detailed report."""
        html = f"""
<!DOCTYPE html>
<html>
<head>
    <title>EnviroNet Analyzer - Test Results Report</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .header {{ background-color: #f0f0f0; padding: 20px; border-radius: 5px; }}
        .summary {{ background-color: #e8f5e8; padding: 15px; border-radius: 5px; margin: 20px 0; }}
        .suite {{ background-color: #f9f9f9; padding: 15px; margin: 10px 0; border-radius: 5px; }}
        .success {{ color: green; }}
        .warning {{ color: orange; }}
        .error {{ color: red; }}
        .memory {{ background-color: #fff3cd; padding: 15px; border-radius: 5px; }}
        table {{ border-collapse: collapse; width: 100%; }}
        th, td {{ border: 1px solid #ddd; padding: 8px; text-align: left; }}
        th {{ background-color: #f2f2f2; }}
    </style>
</head>
<body>
    <div class="header">
        <h1>EnviroNet Analyzer - Test Results Report</h1>
        <p>Generated on: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
    </div>
"""
        
        # Add summary section
        total_tests = sum(r.get('total_tests', 0) for r in self.results.values() if isinstance(r, dict) and 'total_tests' in r)
        total_failures = sum(r.get('total_failures', 0) for r in self.results.values() if isinstance(r, dict) and 'total_failures' in r)
        total_errors = sum(r.get('total_errors', 0) for r in self.results.values() if isinstance(r, dict) and 'total_errors' in r)
        total_time = sum(r.get('total_time', 0.0) for r in self.results.values() if isinstance(r, dict) and 'total_time' in r)
        
        if total_tests > 0:
            overall_success_rate = ((total_tests - total_failures - total_errors) / total_tests) * 100
        else:
            overall_success_rate = 0
        
        html += f"""
    <div class="summary">
        <h2>Test Summary</h2>
        <table>
            <tr><th>Metric</th><th>Value</th></tr>
            <tr><td>Total Tests</td><td>{total_tests}</td></tr>
            <tr><td>Failures</td><td class="error">{total_failures}</td></tr>
            <tr><td>Errors</td><td class="error">{total_errors}</td></tr>
            <tr><td>Success Rate</td><td class="success">{overall_success_rate:.1f}%</td></tr>
            <tr><td>Total Time</td><td>{total_time:.2f}s</td></tr>
        </table>
    </div>
"""
        
        # Add memory analysis
        if 'memory' in self.results:
            mem = self.results['memory']
            html += f"""
    <div class="memory">
        <h2>Memory Analysis</h2>
        <table>
            <tr><th>Type</th><th>Bytes</th></tr>
            <tr><td>Definitely Lost</td><td class="error">{mem['definitely_lost']}</td></tr>
            <tr><td>Indirectly Lost</td><td class="warning">{mem['indirectly_lost']}</td></tr>
            <tr><td>Possibly Lost</td><td class="warning">{mem['possibly_lost']}</td></tr>
            <tr><td>Still Reachable</td><td class="success">{mem['still_reachable']}</td></tr>
            <tr><td>Total Leaks</td><td class="error">{mem['total_leaks']}</td></tr>
        </table>
    </div>
"""
        
        # Add test suite details
        html += """
    <h2>Test Suite Details</h2>
"""
        
        for name, result in self.results.items():
            if name == 'memory':
                continue
            
            if isinstance(result, dict) and 'suite_results' in result:
                html += f"""
    <div class="suite">
        <h3>{name}</h3>
        <table>
            <tr><th>Suite</th><th>Tests</th><th>Success Rate</th><th>Time (s)</th></tr>
"""
                
                for suite_name, suite_result in result['suite_results'].items():
                    status_class = "success" if suite_result['success_rate'] == 100 else "warning" if suite_result['success_rate'] > 80 else "error"
                    html += f"""
            <tr>
                <td>{suite_name}</td>
                <td>{suite_result['tests']}</td>
                <td class="{status_class}">{suite_result['success_rate']:.1f}%</td>
                <td>{suite_result['time']:.2f}</td>
            </tr>
"""
                
                html += """
        </table>
    </div>
"""
        
        html += """
</body>
</html>
"""
        
        return html
    
    def generate_charts(self, output_dir=None):
        """Generate charts and visualizations of test results."""
        if not self.results:
            print("No test results to visualize")
            return
        
        if output_dir is None:
            output_dir = self.test_results_dir
        
        output_dir = Path(output_dir)
        output_dir.mkdir(exist_ok=True)
        
        try:
            # Success rate chart
            self.plot_success_rates(output_dir)
            
            # Test time distribution
            self.plot_test_times(output_dir)
            
            # Memory usage chart
            if 'memory' in self.results:
                self.plot_memory_usage(output_dir)
                
        except ImportError:
            print("Matplotlib not available, skipping chart generation")
        except Exception as e:
            print(f"Error generating charts: {e}")
    
    def plot_success_rates(self, output_dir):
        """Plot success rates for different test suites."""
        suites = []
        success_rates = []
        
        for name, result in self.results.items():
            if name == 'memory':
                continue
            
            if isinstance(result, dict) and 'suite_results' in result:
                for suite_name, suite_result in result['suite_results'].items():
                    suites.append(suite_name)
                    success_rates.append(suite_result['success_rate'])
        
        if not suites:
            return
        
        plt.figure(figsize=(12, 6))
        bars = plt.bar(range(len(suites)), success_rates, color=['green' if rate == 100 else 'orange' if rate > 80 else 'red' for rate in success_rates])
        plt.xlabel('Test Suite')
        plt.ylabel('Success Rate (%)')
        plt.title('Test Suite Success Rates')
        plt.xticks(range(len(suites)), suites, rotation=45, ha='right')
        plt.ylim(0, 100)
        
        # Add value labels on bars
        for bar, rate in zip(bars, success_rates):
            plt.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1, f'{rate:.1f}%', ha='center', va='bottom')
        
        plt.tight_layout()
        plt.savefig(output_dir / 'success_rates.png', dpi=300, bbox_inches='tight')
        plt.close()
    
    def plot_test_times(self, output_dir):
        """Plot test execution times."""
        suites = []
        times = []
        
        for name, result in self.results.items():
            if name == 'memory':
                continue
            
            if isinstance(result, dict) and 'suite_results' in result:
                for suite_name, suite_result in result['suite_results'].items():
                    suites.append(suite_name)
                    times.append(suite_result['time'])
        
        if not suites:
            return
        
        plt.figure(figsize=(12, 6))
        plt.bar(range(len(suites)), times, color='skyblue')
        plt.xlabel('Test Suite')
        plt.ylabel('Execution Time (s)')
        plt.title('Test Suite Execution Times')
        plt.xticks(range(len(suites)), suites, rotation=45, ha='right')
        
        # Add value labels on bars
        for i, time in enumerate(times):
            plt.text(i, time + 0.01, f'{time:.2f}s', ha='center', va='bottom')
        
        plt.tight_layout()
        plt.savefig(output_dir / 'test_times.png', dpi=300, bbox_inches='tight')
        plt.close()
    
    def plot_memory_usage(self, output_dir):
        """Plot memory usage analysis."""
        mem = self.results['memory']
        
        labels = ['Definitely Lost', 'Indirectly Lost', 'Possibly Lost', 'Still Reachable']
        sizes = [mem['definitely_lost'], mem['indirectly_lost'], mem['possibly_lost'], mem['still_reachable']]
        colors = ['red', 'orange', 'yellow', 'green']
        
        # Filter out zero values
        non_zero_labels = []
        non_zero_sizes = []
        non_zero_colors = []
        
        for label, size, color in zip(labels, sizes, colors):
            if size > 0:
                non_zero_labels.append(label)
                non_zero_sizes.append(size)
                non_zero_colors.append(color)
        
        if not non_zero_sizes:
            return
        
        plt.figure(figsize=(10, 8))
        plt.pie(non_zero_sizes, labels=non_zero_labels, colors=non_zero_colors, autopct='%1.1f%%', startangle=90)
        plt.title('Memory Usage Analysis')
        plt.axis('equal')
        plt.savefig(output_dir / 'memory_usage.png', dpi=300, bbox_inches='tight')
        plt.close()

def main():
    parser = argparse.ArgumentParser(description='Analyze EnviroNet Analyzer test results')
    parser.add_argument('--results-dir', default='build', help='Directory containing test results')
    parser.add_argument('--config', default='tests/test_configs.json', help='Test configuration file')
    parser.add_argument('--output-dir', help='Output directory for reports and charts')
    parser.add_argument('--html-report', help='Generate HTML report')
    parser.add_argument('--charts', action='store_true', help='Generate charts and visualizations')
    
    args = parser.parse_args()
    
    # Create analyzer
    analyzer = TestAnalyzer(args.results_dir, args.config)
    
    # Collect and analyze results
    print("Collecting test results...")
    analyzer.collect_results()
    
    if not analyzer.results:
        print("No test results found. Make sure to run tests first.")
        sys.exit(1)
    
    # Generate summary report
    analyzer.generate_summary_report()
    
    # Generate detailed HTML report
    if args.html_report:
        analyzer.generate_detailed_report(args.html_report)
    else:
        analyzer.generate_detailed_report()
    
    # Generate charts
    if args.charts:
        print("\nGenerating charts...")
        analyzer.generate_charts(args.output_dir)
    
    print("\nAnalysis complete!")

if __name__ == "__main__":
    main()
