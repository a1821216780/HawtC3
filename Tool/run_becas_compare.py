"""
Run all BECAS cases with BECASresults.txt through Qahse and compare results.
Also runs all other cases and outputs a summary.
"""

import os
import re
import subprocess
import sys

EXE = r"E:\Qahse\build\release\Qahse.exe"
BASE_DIR = r"E:\Qahse\Demo\PCSL_Becas_Compare"

# Cases with BECAS reference results
REF_CASES = ["S1", "C1", "C2", "C4", "S2_1", "S3_0", "S3_22.5", "S3_45"]

# All cases to run (ordered by complexity)
ALL_CASES = [
    # Reference cases first
    "S1", "C1", "C2", "C4", "S2_1", "S3_0", "S3_22.5", "S3_45",
    # Scaling variants
    "S2_10", "S2_100", "S2_1000", "S2_10000", "S2_100000",
    "C3_1", "C3_10", "C3_100", "C3_1000", "C3_10000", "C3_100000",
    # Additional angle variants
    "S3_67.5", "S3_90",
    # Other Q4 cases
    "S4", "Square Abaqus 10x10", "Square Abaqus 10x10 Ortho", "Square Abaqus 2x2",
    "T1", "T2", "WTAirfoil",
    # T6 and Q8 cases
    "S1_T6", "S1_T6_fine",
    "W2", "W3", "T3",
    "S1_Q8_fine",
    # Large cases
    "BECAS_SECTION", "Detailed WT Section",
]


def run_case(case_name):
    """Run a single case through Qahse, return stdout"""
    case_dir = os.path.join(BASE_DIR, case_name)
    pcs_file = os.path.join(case_dir, f"{case_name}.pcs")
    if not os.path.exists(pcs_file):
        return None, f"PCS file not found: {pcs_file}"

    try:
        result = subprocess.run(
            [EXE, "--pcsl", pcs_file],
            input="\n",
            capture_output=True, text=True, timeout=120,
            encoding="utf-8", errors="replace"
        )
        return result.stdout + result.stderr, None
    except subprocess.TimeoutExpired:
        return None, "TIMEOUT"
    except Exception as e:
        return None, str(e)


def parse_ks_from_output(text):
    """Parse 6x6 stiffness matrix from Qahse output"""
    # Find all text after "Cross-Section Stiffness Matrix" and before next section
    idx = text.find("Cross-Section Stiffness Matrix")
    if idx < 0:
        return None
    rest = text[idx:]
    # Find end marker
    end_idx = rest.find("Elastic Center:")
    if end_idx > 0:
        rest = rest[:end_idx]
    # Extract all scientific notation numbers
    vals = re.findall(r'[+-]?\d+\.\d+e[+-]\d+', rest)
    if len(vals) < 36:
        return None
    ks = [[0.0]*6 for _ in range(6)]
    for i in range(6):
        for j in range(6):
            ks[i][j] = float(vals[i*6 + j])
    return ks


def parse_props_from_output(text):
    """Parse elastic/shear center and mass from output"""
    props = {}
    m = re.search(r'Elastic Center:\s*\(([^,]+),\s*([^)]+)\)', text)
    if m:
        props['ex'] = float(m.group(1))
        props['ey'] = float(m.group(2))
    m = re.search(r'Shear Center:\s*\(([^,]+),\s*([^)]+)\)', text)
    if m:
        props['sx'] = float(m.group(1))
        props['sy'] = float(m.group(2))
    m = re.search(r'Mass/length:\s*([^\s]+)', text)
    if m:
        props['mass'] = float(m.group(1))
    return props


def parse_becas_results(path):
    """Parse BECASresults.txt for K matrix, centers, and mass"""
    with open(path, "r") as f:
        text = f.read()

    result = {}

    # Parse K matrix (6x6)
    km = re.search(r'K=\s*\n((?:.*\n){6})', text)
    if km:
        k_text = km.group(1)
        vals = re.findall(r'[+-]?\d+\.?\d*(?:[eE][+-]?\d+)?', k_text)
        if len(vals) >= 36:
            ks = [[0.0]*6 for _ in range(6)]
            for i in range(6):
                for j in range(6):
                    ks[i][j] = float(vals[i*6 + j])
            result['ks'] = ks

    # Parse centers
    m = re.search(r'ElasticX=\s*([^\s]+)', text)
    if m:
        result['ex'] = float(m.group(1))
    m = re.search(r'ElasticY=\s*([^\s]+)', text)
    if m:
        result['ey'] = float(m.group(1))
    m = re.search(r'ShearX=\s*([^\s]+)', text)
    if m:
        result['sx'] = float(m.group(1))
    m = re.search(r'ShearY=\s*([^\s]+)', text)
    if m:
        result['sy'] = float(m.group(1))
    m = re.search(r'Mass=\s*([^\s]+)', text)
    if m:
        result['mass'] = float(m.group(1))

    return result


def rel_error(a, b):
    """Relative error between two values"""
    if abs(b) < 1e-30:
        if abs(a) < 1e-30:
            return 0.0
        return float('inf')
    return abs(a - b) / abs(b)


def main():
    print("=" * 80)
    print("Qahse PcsL vs BECAS Comparison")
    print("=" * 80)

    results = {}
    failed = []

    for case in ALL_CASES:
        case_dir = os.path.join(BASE_DIR, case)
        if not os.path.isdir(case_dir):
            print(f"  SKIP  {case}: directory not found")
            continue

        print(f"  RUN   {case}...", end="", flush=True)
        output, err = run_case(case)
        if err:
            print(f" FAILED: {err}")
            failed.append((case, err))
            continue

        ks = parse_ks_from_output(output)
        props = parse_props_from_output(output)
        if ks is None:
            print(f" FAILED: could not parse Ks")
            failed.append((case, "parse error"))
            continue

        results[case] = {'ks': ks, 'props': props}
        print(f" OK  Mass={props.get('mass', 0):.6e}  "
              f"Ks11={ks[0][0]:.6e}  Ks33={ks[2][2]:.6e}  Ks44={ks[3][3]:.6e}")

    # Now compare with BECAS reference
    print("\n" + "=" * 80)
    print("BECAS Reference Comparison")
    print("=" * 80)
    print(f"{'Case':<12} {'Param':<10} {'Qahse':>15} {'BECAS':>15} {'RelErr%':>10}")
    print("-" * 62)

    for case in REF_CASES:
        if case not in results:
            print(f"{case:<12} NOT RUN")
            continue

        ref_path = os.path.join(BASE_DIR, case, "BECASresults.txt")
        if not os.path.exists(ref_path):
            continue

        ref = parse_becas_results(ref_path)
        hc = results[case]

        if 'ks' in ref and hc['ks']:
            # Compare K matrix diagonal
            labels = ["Ks11", "Ks22", "Ks33", "Ks44", "Ks55", "Ks66"]
            for i in range(6):
                h_val = hc['ks'][i][i]
                r_val = ref['ks'][i][i]
                err = rel_error(h_val, r_val) * 100
                marker = " *" if err > 1.0 else ""
                print(f"{case:<12} {labels[i]:<10} {h_val:>15.6e} {r_val:>15.6e} {err:>9.4f}%{marker}")

            # Compare key off-diagonals if non-zero
            for i, j in [(2, 4), (4, 2)]:  # Ks(3,5) coupling
                h_val = hc['ks'][i][j]
                r_val = ref['ks'][i][j]
                if abs(r_val) > 1e-20:
                    err = rel_error(h_val, r_val) * 100
                    marker = " *" if err > 1.0 else ""
                    print(f"{case:<12} Ks{i+1}{j+1}      {h_val:>15.6e} {r_val:>15.6e} {err:>9.4f}%{marker}")

        # Mass
        if 'mass' in ref and 'mass' in hc['props']:
            err = rel_error(hc['props']['mass'], ref['mass']) * 100
            marker = " *" if err > 1.0 else ""
            print(f"{case:<12} {'Mass':<10} {hc['props']['mass']:>15.6e} {ref['mass']:>15.6e} {err:>9.4f}%{marker}")

        print()

    # Summary
    print("=" * 80)
    print(f"Total: {len(results)} cases OK, {len(failed)} failed")
    if failed:
        print("Failed cases:")
        for c, e in failed:
            print(f"  {c}: {e}")

    # Check VTK files
    print(f"\nVTK files generated:")
    for case in results:
        vtu = os.path.join(BASE_DIR, case, "Result1", f"{case}.vtu")
        if os.path.exists(vtu):
            size = os.path.getsize(vtu)
            print(f"  {case}: {size/1024:.1f} KB")


if __name__ == "__main__":
    main()
