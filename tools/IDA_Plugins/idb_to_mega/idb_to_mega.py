import os
import idaapi


SKIP_LIST = [
    "unknown",
    # "nullsub_",
    "sub_",
    "loc_",
    "byte_",
    "dword_",
    "word_",
]
MIN_SIG_LENGTH = 30


def is_skipped(name):
    """Return if the function is skipped."""
    return any(name.startswith(prefix) for prefix in SKIP_LIST)


def set_v_bytes(v, pos, length=4):
    """Set vector bytes."""
    for i in range(pos, pos + length):
        v[i] = True


def create_func_signature(start, length):
    """Return function signature in mega format."""
    if length < MIN_SIG_LENGTH:
        return

    ea = start
    end = start + length
    sig = ""
    publics = []
    refs = {}
    v = [False for _ in range(length)]

    while (ea - start < length):
        flags = idaapi.getFlags(ea)
        if idaapi.has_name(flags):
            publics.append(ea)

        ref = idaapi.get_first_dref_from(ea)
        if ref != idaapi.BADADDR:
            ref_loc = ea
            set_v_bytes(v, ref_loc - start)
            refs[ref_loc] = ref

            # Check if there is a second data location ref'd
            ref = idaapi.get_next_dref_from(ea, ref)
            if ref != idaapi.BADADDR:
                ref_loc = ea
                set_v_bytes(v, ref_loc - start)
                refs[ref_loc] = ref
        else:
            # Code ref?
            ref = idaapi.get_first_fcref_from(ea)
            if ref != idaapi.BADADDR:
                if not start <= ref < end:
                    ref_loc = ea
                    set_v_bytes(v, ref_loc - start)
                    refs[ref_loc] = ref

            # Check for r13 and rtoc
            disasm = idaapi.generate_disasm_line(ea)
            if "%r13" in disasm or "%rtoc" in disasm:
                ref_loc = ea
                set_v_bytes(v, ref_loc - start)

        ea = idaapi.next_not_tail(ea)

    line = ""
    for i in range(length):
        if v[i]:
            line += ".."
        else:
            line += "{:02X}".format(idaapi.get_byte(start + i))

    # Write publics
    found = False
    for public in sorted(publics):
        name = idaapi.get_true_name(idaapi.BADADDR, public)
        if name:
            found = True
            if is_skipped(name):
                idaapi.warning("Rename the function {} ({})!".format(
                    name, "it is on the skip list")
                )
                return
            else:
                line += " :{:04X} {}".format(public - start, name)

    if not found:
        idaapi.warning("The function has autogenerated name, rename it first!")

    # Write refs
    for ref_loc, ref in sorted(refs.items()):
        name = idaapi.get_true_name(idaapi.BADADDR, ref)
        if name:
            if not is_skipped(name) and ref_loc != idaapi.BADADDR:
                line += " ^{:04X} {}".format(ref_loc - start, name)

    return line


def process_func(i):
    """Convert function i into a mega file line."""
    func = idaapi.getn_func(i)
    if not func:
        return

    if idaapi.has_name(idaapi.getFlags(func.startEA)) and \
       not (func.flags & idaapi.FUNC_LIB):
        return create_func_signature(func.startEA, func.endEA - func.startEA)


def idb_to_mega(filename):
    """Convert IDB to mega file."""
    mode = "w"
    # File already exists
    if os.path.exists(filename):
        ask = idaapi.askyn_c(False, "Append result to existing file?")
        if ask == -1:
            return
        elif ask == 0:
            if 1 != idaapi.askyn_c(False, "Overwrite existing file?"):
                return
        elif ask == 1:
            mode = "a+"
    # Process functions
    with open(filename, mode) as f:
        for i in range(idaapi.get_func_qty()):
            line = process_func(i)
            if line:
                line += "\n"
                f.write(line)


if __name__ == "__main__":
    """TODO: Add/Append ending: ---"""
    filename = idaapi.askfile_c(True, "*.mega",
                                "Enter the name of the mega file:")
    if not filename:
        idaapi.warning("Save filename not provided!")
    else:
        idb_to_mega(filename)
    idc.Message("idb_to_mega finished!\n")
