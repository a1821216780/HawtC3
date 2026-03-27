"""Check node ID contiguity in BECAS_SECTION and WTAirfoil."""
import os

for case in ["BECAS_SECTION", "WTAirfoil"]:
    path = rf"E:\Qahse\Demo\PCSL_Becas_Compare\{case}\{case}.pcs"
    if not os.path.exists(path):
        print(f"{case}: FILE NOT FOUND")
        continue
    
    lines = open(path, "r", encoding="utf-8").readlines()
    in_nodes = False
    in_elems = False
    node_ids = []
    elem_refs = []
    
    for l in lines:
        s = l.strip()
        if "!BeginNode" in s:
            in_nodes = True
            in_elems = False
            continue
        if "!BeginElement" in s:
            in_nodes = False
            in_elems = True
            continue
        if not s:
            continue
        
        if in_nodes:
            parts = s.split()
            if len(parts) >= 3:
                try:
                    node_ids.append(int(parts[0]))
                except:
                    pass
        
        if in_elems:
            parts = s.split("\t")
            if len(parts) >= 5:
                try:
                    for p in parts[1:5]:
                        elem_refs.append(int(p))
                except:
                    pass
    
    if node_ids:
        min_n = min(node_ids)
        max_n = max(node_ids)
        contiguous = (len(set(node_ids)) == max_n - min_n + 1)
        max_ref = max(elem_refs) if elem_refs else 0
        print(f"{case}:")
        print(f"  Nodes: {len(node_ids)}, IDs: {min_n}..{max_n}")
        print(f"  Contiguous: {contiguous}")
        print(f"  Max element node ref: {max_ref}")
        print(f"  Ref in node set: {max_ref in set(node_ids)}")
        # Check for refs not in node set
        node_set = set(node_ids)
        bad_refs = [r for r in set(elem_refs) if r not in node_set]
        print(f"  Bad refs (not in nodes): {len(bad_refs)}")
        if bad_refs:
            print(f"  Examples: {sorted(bad_refs)[:10]}")
    else:
        print(f"{case}: NO NODES FOUND")
