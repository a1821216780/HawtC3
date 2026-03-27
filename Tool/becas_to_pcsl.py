"""
Convert BECAS test cases (N2D.in, E2D.in, EMAT.in, MATPROPS.in) to PcsL format (.pcs + materials.inp).
Outputs to E:\Qahse\Demo\PCSL_Becas_Compare\<CaseName>\

Usage:
    python becas_to_pcsl.py
"""

import os
import re
import glob
import shutil

BECAS_DIR = r"E:\Qahse\Qahse\Test\PcsL"
OUTPUT_DIR = r"E:\Qahse\Demo\PCSL_Becas_Compare"

# Cases to skip (mixed element types not supported)
SKIP_CASES = {"S1_Q8andT6_fine", "crack_validation", "fatigue", "stress_validation"}


def read_n2d(path):
    """Read N2D.in: nodeId x y (1-based IDs)"""
    nodes = []
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) >= 3:
                nid = int(float(parts[0]))
                x = float(parts[1])
                y = float(parts[2])
                nodes.append((nid, x, y))
    return nodes


def read_e2d(path):
    """Read E2D.in: elemId n1 n2 ... [trailing zeros]
    Returns list of (elemId, [node_ids]) and detected element type.
    """
    elements = []
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) < 5:
                continue
            eid = int(parts[0])
            vals = [int(x) for x in parts[1:]]
            elements.append((eid, vals))

    if not elements:
        return elements, "Q4", 4

    # Detect element type by checking trailing zeros across all elements
    max_cols = max(len(e[1]) for e in elements)

    # Check how many trailing columns are always zero
    # For Q4: 4 nodes + 4 zeros = 8 vals -> 4 trailing zeros
    # For Q8: 8 nodes + 0-1 trailing zero = 8-9 vals
    # For T6: 6 nodes + 2 zeros = 8 vals -> 2 trailing zeros

    # Count non-zero columns from the end
    last_nonzero = 0
    for _, vals in elements:
        for i in range(len(vals)):
            if vals[i] != 0:
                last_nonzero = max(last_nonzero, i + 1)

    if last_nonzero <= 4:
        return elements, "Q4", 4
    elif last_nonzero <= 6:
        return elements, "T6", 6
    else:
        return elements, "Q8", 8


def read_emat(path):
    """Read EMAT.in: elemId matId fiberAngle1 fiberAngle2"""
    emat = {}
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) >= 3:
                eid = int(parts[0])
                mat_id = int(parts[1])
                angle1 = float(parts[2])
                emat[eid] = (mat_id, angle1)
    return emat


def read_matprops(path):
    """Read MATPROPS.in: E1 E2 E3 G12 G13 G23 nu12 nu13 nu23 rho (one line per material)"""
    materials = []
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) >= 10:
                props = []
                for p in parts[:10]:
                    # Handle scientific notation like 48E9, 100E-3
                    props.append(float(p))
                materials.append(props)
    return materials


def write_materials_inp(path, materials):
    """Write materials.inp in PcsL format"""
    with open(path, "w", encoding="utf-8") as f:
        f.write("Id   E1       E2         E3      G12   G13  G23    Nu12      Nu13      Nu23    Density      Name\n")
        f.write(" (-)   (Pa)     (Pa)       (Pa)    (Pa)  (Pa) (Pa)   (-)       (-)       (-)     (kg/m3)      (-)\n")
        f.write("!这一行必须有\n")
        for i, m in enumerate(materials):
            mat_id = i + 1  # 1-based
            # E1 E2 E3 G12 G13 G23 nu12 nu13 nu23 rho
            f.write(f"{mat_id}  {m[0]:.15g}  {m[1]:.15g}  {m[2]:.15g}  "
                    f"{m[3]:.15g}  {m[4]:.15g}  {m[5]:.15g}  "
                    f"{m[6]:.15g}  {m[7]:.15g}  {m[8]:.15g}  "
                    f"{m[9]:.15g}  #Mat{mat_id}\n")


def elem_type_code(etype_str):
    """Convert element type string to PcsL code"""
    if etype_str == "Q4":
        return 0
    elif etype_str == "Q8":
        return 1
    elif etype_str == "T6":
        return 3
    return 0


def write_pcs_file(path, case_name, nodes, elements, emat, etype_str, nnpe, nmat):
    """Write .pcs file in PcsL SelfMesh format"""
    nn = len(nodes)
    ne = len(elements)

    with open(path, "w", encoding="utf-8") as f:
        f.write("********************  PCSL 主输入文件 **********************\n")
        f.write(f"BECAS Test Case: {case_name}\n")
        f.write("!这个空行必须要\n")
        f.write("-------------------------------- 基本信息 -----------------------------------------------\n")
        f.write(f"1.0      Bl_length        -    叶片总长（米）\n")
        f.write(f"0        N_sections       -    叶片截面总数\n")
        f.write(f"{nmat}        N_materials      -    材料数量\n")
        f.write(f"3        Out_format       -    输出格式\n")
        f.write(f"true     TabDelim         -    制表符分隔\n")
        f.write(f"true     FEMSove          -    启用FEM求解\n")
        f.write(f'"./materials.inp"     MaterialsPath  - 材料文件路径\n')
        f.write("-------------------------------- FEM 方法参数 -----------------------------------------------\n")
        f.write(f"true     SelfMesh         -    自定义网格\n")
        f.write(f"true     Shear_Center     -    剪切中心转换\n")
        f.write(f"{elem_type_code(etype_str)}        ElememtType      -    单元类型\n")
        f.write(f"true     VTKShow          -    输出VTK\n")
        f.write(f"true     SaveCatch        -    保存网格缓存\n")
        f.write(f"true     CalStress        -    应力计算\n")
        f.write("\n")
        f.write("======  Outputs  =============================================\n")
        f.write(f'"./Result1/"          SumPath   - 输出文件夹\n')
        f.write("\n")
        f.write("-------------------------------- 自定义网格 ----------------------------------------\n")
        f.write(f"{nn}        FEMNodeNum       -    节点数量\n")
        f.write(f"{ne}        FEMElementNum    -    单元数量\n")

        # Build remapping: original 1-based BECAS ID -> contiguous 0-based index
        node_id_map = {}
        for idx, (nid, x, y) in enumerate(nodes):
            node_id_map[nid] = idx

        # Write nodes (contiguous 0-based IDs)
        f.write("!BeginNode  (节点编号，从零开始 ； 节点x坐标  ；节点y坐标)\n")
        for nid, x, y in nodes:
            f.write(f"{node_id_map[nid]}\t{x:.15g}\t{y:.15g}\n")

        f.write("\n")

        # Write elements (contiguous 0-based IDs, with matId and angle from EMAT)
        if nnpe == 4:
            f.write("!BeginElement(单元 ； 节点1 ； 节点2 ； 节点3 ； 节点4 ；材料编号 ；铺层角度)\n")
        elif nnpe == 8:
            f.write("!BeginElement(单元 ； 节点1-8 ；材料编号 ；铺层角度)\n")
        elif nnpe == 6:
            f.write("!BeginElement(单元 ； 节点1-6 ；材料编号 ；铺层角度)\n")

        for eid, node_vals in elements:
            new_eid = eid - 1  # Convert to 0-based
            # Get material and angle from EMAT
            mat_id, angle = emat.get(eid, (1, 0.0))

            # Remap node IDs to contiguous 0-based via node_id_map
            node_ids = [node_id_map[v] for v in node_vals[:nnpe]]

            node_str = "\t".join(str(n) for n in node_ids)
            f.write(f"{new_eid}\t{node_str}\t{mat_id}\t{angle:g}\n")


def convert_case(case_dir, case_name, output_base):
    """Convert a single BECAS case to PcsL format"""
    n2d_path = os.path.join(case_dir, "N2D.in")
    e2d_path = os.path.join(case_dir, "E2D.in")
    emat_path = os.path.join(case_dir, "EMAT.in")
    matprops_path = os.path.join(case_dir, "MATPROPS.in")

    # Check all files exist
    for p in [n2d_path, e2d_path, emat_path, matprops_path]:
        if not os.path.exists(p):
            print(f"  SKIP {case_name}: missing {os.path.basename(p)}")
            return False

    # Read BECAS inputs
    nodes = read_n2d(n2d_path)
    elements, etype_str, nnpe = read_e2d(e2d_path)
    emat = read_emat(emat_path)
    materials = read_matprops(matprops_path)

    if not nodes or not elements or not materials:
        print(f"  SKIP {case_name}: empty data")
        return False

    nmat = len(materials)

    # Create output directory
    out_dir = os.path.join(output_base, case_name)
    os.makedirs(out_dir, exist_ok=True)
    os.makedirs(os.path.join(out_dir, "Result1"), exist_ok=True)

    # Write materials.inp
    write_materials_inp(os.path.join(out_dir, "materials.inp"), materials)

    # Write .pcs file
    pcs_name = f"{case_name}.pcs"
    write_pcs_file(
        os.path.join(out_dir, pcs_name),
        case_name, nodes, elements, emat, etype_str, nnpe, nmat
    )

    # Copy BECASresults.txt if exists
    becas_ref = os.path.join(case_dir, "BECASresults.txt")
    if os.path.exists(becas_ref):
        shutil.copy2(becas_ref, os.path.join(out_dir, "BECASresults.txt"))

    print(f"  OK   {case_name}: {len(nodes)} nodes, {len(elements)} elements, "
          f"{nmat} materials, {etype_str}")
    return True


def main():
    print(f"BECAS -> PcsL Converter")
    print(f"Source: {BECAS_DIR}")
    print(f"Output: {OUTPUT_DIR}")
    print()

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    case_dirs = sorted(
        [d for d in os.listdir(BECAS_DIR)
         if os.path.isdir(os.path.join(BECAS_DIR, d)) and d not in SKIP_CASES],
        key=str.lower
    )

    ok_count = 0
    skip_count = 0

    for case_name in case_dirs:
        case_dir = os.path.join(BECAS_DIR, case_name)
        if convert_case(case_dir, case_name, OUTPUT_DIR):
            ok_count += 1
        else:
            skip_count += 1

    print(f"\nDone: {ok_count} cases converted, {skip_count} skipped")


if __name__ == "__main__":
    main()
