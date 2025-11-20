#!/usr/bin/env python3

import csv
import argparse
import sys
from pathlib import Path


# ANSI COLORS
GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
RESET = "\033[0m"


def read_type_file(filepath):
    types = {}
    with open(filepath, 'r') as f:
        for line in f:
            if line.strip() and not line.startswith('//'):
                id, type_info = line.strip().split('\t')
                types[id] = type_info
    return types


def split_type_terms(type_str):
    """Split a type string into its constituent terms."""
    import re
    return re.findall(r'\$?[A-Za-z0-9]+|\[|\]|\(|\)|,', type_str)


def compare_types(actual_file, expected_file, verbose=False):
    """
    Return (passed: bool, diffs_output: list[str]).
    Verbose mode prints diffs immediately.
    """
    actual = read_type_file(actual_file)
    expected = read_type_file(expected_file)

    all_keys = sorted(set(actual.keys()) | set(expected.keys()))
    diffs_output = []

    for key in all_keys:
        actual_type = actual.get(key, "MISSING")
        expected_type = expected.get(key, "MISSING")

        if actual_type != expected_type:
            diff_text = []
            diff_text.append(f"\nID: {key}")
            diff_text.append(f"Expected: {expected_type}")
            diff_text.append(f"Actual:   {actual_type}")

            if actual_type != "MISSING" and expected_type != "MISSING":
                actual_terms = split_type_terms(actual_type)
                expected_terms = split_type_terms(expected_type)

                actual_markers = [
                    ('^' * len(a)) if a != e else (' ' * len(a))
                    for a, e in zip(actual_terms, expected_terms)
                ]

                pointer = ''
                pos = 0
                for term, marker in zip(actual_terms, actual_markers):
                    spaces_needed = actual_type.find(term, pos) - pos
                    pointer += ' ' * spaces_needed + marker
                    pos = actual_type.find(term, pos) + len(term)

                diff_text.append(f"          {pointer}")

            diff_text.append("-" * 80)
            diffs_output.append("\n".join(diff_text))

    passed = len(diffs_output) == 0

    # Verbose prints everything immediately
    if verbose and not passed:
        print(f"\n{YELLOW}=== Comparing ==={RESET}")
        print(f"Actual:   {actual_file}")
        print(f"Expected: {expected_file}")
        print("=" * 80)
        print("\n".join(diffs_output))

    return passed, diffs_output


def resolve_path(root: Path, p: str) -> Path:
    ppath = Path(p)
    if not ppath.is_absolute():
        ppath = root / ppath
    try:
        return ppath.resolve()
    except Exception:
        return ppath


def run_recursive(folder: Path, verbose=False):
    """Recursively find folders with matches and run tests."""
    if verbose:
        print(f"{YELLOW}Searching recursively in:{RESET} {folder}\n")

    matched_folders = []

    # Alphabetical traversal
    for dirpath in sorted(folder.rglob("*")):
        if not dirpath.is_dir():
            continue

        actual = dirpath / "inferred_type.csv"
        expected = dirpath / "inferred_type - expected.csv"

        if actual.exists() and expected.exists():
            matched_folders.append(dirpath)

    if not matched_folders:
        print("No matching test pairs found.\n")
        return

    total = len(matched_folders)
    passed_count = 0
    failed_count = 0

    for dirpath in matched_folders:
        if verbose:
            print(f"\n----- Running test in folder {dirpath} -----")

        passed, diffs = compare_types(
            dirpath / "inferred_type.csv",
            dirpath / "inferred_type - expected.csv",
            verbose=verbose
        )

        if passed:
            passed_count += 1
            if not verbose:
                print(f"{dirpath}: {GREEN}PASS{RESET}")
        else:
            failed_count += 1
            if not verbose:
                print(f"{dirpath}: {RED}FAIL{RESET}")
                print("\n".join(diffs))

    # Summary
    print("\n" + "=" * 60)
    print(f"{YELLOW}Summary:{RESET}")
    print(f"Total tests: {total}")
    print(f"{GREEN}Passed: {passed_count}{RESET}")
    print(f"{RED}Failed: {failed_count}{RESET}")
    print("=" * 60 + "\n")


if __name__ == "__main__":
    samples_root = Path(__file__).parent

    parser = argparse.ArgumentParser(
        description="Compare type files or recurse through folders."
    )

    parser.add_argument("actual", nargs="?", help="Actual file, or omit with --folder")
    parser.add_argument("expected", nargs="?", help="Expected file, or omit with --folder")

    parser.add_argument("--folder", "-f",
                        help="Recursively check all subfolders for inferred_type.csv and inferred_type - expected.csv")

    parser.add_argument("--verbose", "-v", action="store_true",
                        help="Enable detailed output.")

    args = parser.parse_args()

    # Folder mode
    if args.folder:
        folder = resolve_path(samples_root, args.folder)
        if not folder.exists() or not folder.is_dir():
            print(f"Folder not found or not a directory: {folder}")
            sys.exit(2)
        run_recursive(folder, verbose=args.verbose)
        sys.exit(0)

    # Two-file mode
    if not args.actual or not args.expected:
        print("Error: You must provide two files OR --folder.\n")
        parser.print_help()
        sys.exit(2)

    actual_file = resolve_path(samples_root, args.actual)
    expected_file = resolve_path(samples_root, args.expected)

    errors = []
    if not actual_file.exists():
        errors.append(f"Actual file not found: {actual_file}")
    if not expected_file.exists():
        errors.append(f"Expected file not found: {expected_file}")

    if errors:
        print("Input error(s):")
        for e in errors:
            print("  " + e)
        sys.exit(2)

    passed, diffs = compare_types(actual_file, expected_file, verbose=args.verbose)

    if passed:
        print(f"{GREEN}PASS{RESET}")
    else:
        print(f"{RED}FAIL{RESET}")
        if not args.verbose:
            print("\n".join(diffs))
