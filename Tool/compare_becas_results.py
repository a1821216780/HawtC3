"""
Parse Qahse output files and compare with BECAS reference results.
Reads SectionResult_*.out files from Result1/ directories.
"""

import os
import re

BASE_DIR = r"E:\Qahse\Demo\PCSL_Becas_Compare"

# Cases with BECAS reference results
REF_CASES = ["S1", "C1", "C2", "C4", "S2_1", "S3_0", "S3_22.5", "S3_45"]


def parse_output_file(path):
    """Parse SectionResult_*.out file for Ks matrix and properties"""
    if not os.path.exists(path):
        return None

    with open(path, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    result = {}

    # Parse Ks matrix (transformed, may be at shear center)
    idx = text.find("Cross-Section Stiffness Matrix (Ks):")
    if idx >= 0:
        end = text.find("Cross-Section Stiffness Matrix at Origin", idx)
        if end < 0:
            end = text.find("Cross-Section Mass Matrix", idx)
        if end < 0:
            end = len(text)
        ks_text = text[idx:end]
        vals = re.findall(r'[+-]?\d+\.\d+e[+-]\d+', ks_text)
        if len(vals) >= 36:
            ks = [[0.0]*6 for _ in range(6)]
            for i in range(6):
                for j in range(6):
                    ks[i][j] = float(vals[i*6 + j])
            result['ks'] = ks

    # Parse Ks_origin matrix (at origin, for BECAS comparison)
    idx2 = text.find("Cross-Section Stiffness Matrix at Origin (Ks_origin):")
    if idx2 >= 0:
        end2 = text.find("Cross-Section Mass Matrix", idx2)
        if end2 < 0:
            end2 = len(text)
        kso_text = text[idx2:end2]
        vals2 = re.findall(r'[+-]?\d+\.\d+e[+-]\d+', kso_text)
        if len(vals2) >= 36:
            kso = [[0.0]*6 for _ in range(6)]
            for i in range(6):
                for j in range(6):
                    kso[i][j] = float(vals2[i*6 + j])
            result['ks_origin'] = kso

    # Parse properties
    m = re.search(r'Elastic Center X\s*:\s*([^\s]+)', text)
    if m: result['ex'] = float(m.group(1))
    m = re.search(r'Elastic Center Y\s*:\s*([^\s]+)', text)
    if m: result['ey'] = float(m.group(1))
    m = re.search(r'Shear Center X\s*:\s*([^\s]+)', text)
    if m: result['sx'] = float(m.group(1))
    m = re.search(r'Shear Center Y\s*:\s*([^\s]+)', text)
    if m: result['sy'] = float(m.group(1))
    m = re.search(r'Mass per unit length:\s*([^\s]+)', text)
    if m: result['mass'] = float(m.group(1))

    return result


def parse_becas_results(path, case_name=""):
    """Parse BECASresults.txt, handling multi-section files (e.g. S3 with theta=0/22.5/45).
    For multi-section files, selects the section matching the case suffix (e.g. S3_45 -> theta=45)."""
    with open(path, "r") as f:
        text = f.read()

    # Check if multi-section: split by separator lines
    sections = re.split(r'\*{20,}\**\s*', text)
    sections = [s.strip() for s in sections if s.strip()]

    # Determine which section to use
    section_text = text  # default: full text (single section)
    if len(sections) > 1 and case_name:
        # Extract suffix after last underscore (e.g. "S3_45" -> "45", "S3_22.5" -> "22.5", "S2_1" -> "1")
        suffix = case_name.rsplit("_", 1)[-1] if "_" in case_name else ""
        for sec in sections:
            # Match theta=X or alpha=X in section header
            m = re.search(r'(?:theta|alpha)\s*=\s*([^\s]+)', sec, re.IGNORECASE)
            if m and m.group(1).strip() == suffix:
                section_text = sec
                break

    result = {}

    # Parse K matrix (6x6)
    km = re.search(r'K=\s*\n([\s\S]*?)(?:\n\s*\n|\nM=)', section_text)
    if km:
        k_text = km.group(1)
        vals = re.findall(r'[+-]?\d+\.?\d*(?:[eE][+-]?\d+)?', k_text)
        if len(vals) >= 36:
            ks = [[0.0]*6 for _ in range(6)]
            for i in range(6):
                for j in range(6):
                    ks[i][j] = float(vals[i*6 + j])
            result['ks'] = ks

    m = re.search(r'ElasticX=\s*([^\s]+)', section_text)
    if m: result['ex'] = float(m.group(1))
    m = re.search(r'ElasticY=\s*([^\s]+)', section_text)
    if m: result['ey'] = float(m.group(1))
    m = re.search(r'ShearX=\s*([^\s]+)', section_text)
    if m: result['sx'] = float(m.group(1))
    m = re.search(r'ShearY=\s*([^\s]+)', section_text)
    if m: result['sy'] = float(m.group(1))
    m = re.search(r'Mass=\s*([^\s]+)', section_text)
    if m: result['mass'] = float(m.group(1))

    return result


def rel_error(a, b):
    if abs(b) < 1e-30:
        return 0.0 if abs(a) < 1e-30 else float('inf')
    return abs(a - b) / abs(b)


def main():
    print("=" * 90)
    print("Qahse PcsL vs BECAS Comparison (reading output files)")
    print("=" * 90)

    # Scan all cases
    all_cases = sorted([d for d in os.listdir(BASE_DIR) if os.path.isdir(os.path.join(BASE_DIR, d))], key=str.lower)

    ok_cases = []
    failed_cases = []

    print(f"\n{'Case':<26} {'Status':<8} {'Nodes':>6} {'Elems':>6} {'Mass':>14} {'Ks33(EA)':>14} {'Ks44(EI)':>14}")
    print("-" * 90)

    for case in all_cases:
        case_dir = os.path.join(BASE_DIR, case)
        out_file = os.path.join(case_dir, "Result1", f"SectionResult_{case}.out")
        vtu_file = os.path.join(case_dir, "Result1", f"{case}.vtu")

        if not os.path.exists(out_file):
            # Check if PCS exists
            pcs_file = os.path.join(case_dir, f"{case}.pcs")
            if os.path.exists(pcs_file):
                failed_cases.append(case)
                print(f"{case:<26} {'FAIL':<8}")
            continue

        result = parse_output_file(out_file)
        if result is None or 'ks' not in result:
            failed_cases.append(case)
            print(f"{case:<26} {'PARSE':<8}")
            continue

        ks = result['ks']
        mass = result.get('mass', 0)
        vtk = "VTK" if os.path.exists(vtu_file) else "   "

        ok_cases.append(case)
        print(f"{case:<26} {'OK '+vtk:<8} {'':>6} {'':>6} {mass:>14.6e} {ks[2][2]:>14.6e} {ks[3][3]:>14.6e}")

    # Detailed BECAS comparison
    print(f"\n{'=' * 90}")
    print("Detailed BECAS Reference Comparison (8 cases with BECASresults.txt)")
    print(f"{'=' * 90}")
    print(f"{'Case':<12} {'Parameter':<10} {'Qahse':>16} {'BECAS':>16} {'Err%':>10} {'Status':>8}")
    print("-" * 72)

    for case in REF_CASES:
        case_dir = os.path.join(BASE_DIR, case)
        out_file = os.path.join(case_dir, "Result1", f"SectionResult_{case}.out")
        ref_file = os.path.join(case_dir, "BECASresults.txt")

        if not os.path.exists(out_file) or not os.path.exists(ref_file):
            print(f"{case:<12} --- NOT AVAILABLE ---")
            continue

        hc = parse_output_file(out_file)
        ref = parse_becas_results(ref_file, case)

        if hc is None or ref is None or 'ks' not in ref:
            print(f"{case:<12} --- PARSE ERROR ---")
            continue

        # Use Ks_origin for BECAS comparison (BECAS K is at origin)
        hc_ks = hc.get('ks_origin', hc.get('ks'))
        if hc_ks is None:
            print(f"{case:<12} --- NO Ks DATA ---")
            continue

        labels = ["Ks11", "Ks22", "Ks33", "Ks44", "Ks55", "Ks66"]
        max_err = 0
        for i in range(6):
            h_val = hc_ks[i][i]
            r_val = ref['ks'][i][i]
            err = rel_error(h_val, r_val) * 100
            max_err = max(max_err, err if err != float('inf') else 0)
            status = "PASS" if err < 1.0 else ("WARN" if err < 5.0 else "FAIL")
            print(f"{case:<12} {labels[i]:<10} {h_val:>16.6e} {r_val:>16.6e} {err:>9.4f}% {status:>8}")

        # Key off-diagonal terms (coupling)
        for i, j in [(0, 2), (2, 4), (1, 5), (3, 5)]:
            h_val = hc_ks[i][j]
            r_val = ref['ks'][i][j]
            if abs(r_val) > 1e-10:
                err = rel_error(h_val, r_val) * 100
                status = "PASS" if err < 1.0 else ("WARN" if err < 5.0 else "FAIL")
                print(f"{case:<12} {'Ks'+str(i+1)+str(j+1):<10} {h_val:>16.6e} {r_val:>16.6e} {err:>9.4f}% {status:>8}")

        # Mass
        if 'mass' in hc and 'mass' in ref:
            err = rel_error(hc['mass'], ref['mass']) * 100
            status = "PASS" if err < 1.0 else ("WARN" if err < 5.0 else "FAIL")
            print(f"{case:<12} {'Mass':<10} {hc['mass']:>16.6e} {ref['mass']:>16.6e} {err:>9.4f}% {status:>8}")

        # Centers
        for label, hk, rk in [("ElasticX", 'ex', 'ex'), ("ElasticY", 'ey', 'ey'),
                               ("ShearX", 'sx', 'sx'), ("ShearY", 'sy', 'sy')]:
            if hk in hc and rk in ref:
                h_val = hc[hk]
                r_val = ref[rk]
                if abs(r_val) > 1e-10:
                    err = rel_error(h_val, r_val) * 100
                    status = "PASS" if err < 5.0 else "WARN"
                    print(f"{case:<12} {label:<10} {h_val:>16.6e} {r_val:>16.6e} {err:>9.4f}% {status:>8}")

        print()

    # Summary
    print("=" * 90)
    print(f"Summary: {len(ok_cases)} cases computed, {len(failed_cases)} failed")
    if failed_cases:
        print(f"Failed: {', '.join(failed_cases)}")

    # VTK file summary
    vtk_count = 0
    for case in ok_cases:
        vtu = os.path.join(BASE_DIR, case, "Result1", f"{case}.vtu")
        if os.path.exists(vtu):
            vtk_count += 1
    print(f"VTK files generated: {vtk_count}/{len(ok_cases)}")


if __name__ == "__main__":
    main()
