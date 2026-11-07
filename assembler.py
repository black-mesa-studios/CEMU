#!/usr/bin/env python3

import argparse
import struct
import sys


MAGIC = 0x6734
INSTRUCTION_SIZE = 6
MAX_INSTRUCTIONS = 65536


OPCODES = {
    "NOP":  0x00,
    "STP":  0x01,
    "MOV":  0x02,
    "ADD":  0x03,
    "SUB":  0x04,
    "SHR":  0x05,
    "SHL":  0x06,
    "XOR":  0x07,
    "OR":   0x08,
    "NOT":  0x09,
    "AND":  0x0A,
    "JMP":  0x0B,
    "JZ":   0x0C,
    "CMP":  0x0D,
    "MW":   0x0E,
    "MR":   0x0F,
    "LDI":  0x10,
    "MLDI": 0x12,
    "MSG":  0x11,
    "MUL":  0x13,
    "DIV":  0x14,
    "MOD":  0x15,
    "INC":  0x16,
    "DEC":  0x17,
}


OPERAND_COUNT = {
    "NOP":  0,
    "STP":  0,

    "MOV":  2,
    "ADD":  2,
    "SUB":  2,
    "MUL":  2,
    "DIV":  2,
    "MOD":  2,

    "INC":  1,
    "DEC":  1,

    "SHR":  1,
    "SHL":  1,

    "XOR":  2,
    "OR":   2,
    "NOT":  1,
    "AND":  2,

    "JMP":  1,
    "JZ":   1,

    "CMP":  2,

    "MW":   2,
    "MR":   2,

    "LDI":  2,
    "MLDI": 2,

    "MSG":  2,
}


REGISTER_INSTRUCTIONS = {
    "MOV": (0, 1),
    "ADD": (0, 1),
    "MUL": (0, 1),
    "DIV": (0, 1),
    "MOD": (0, 1),
    "SUB": (0, 1),
    "XOR": (0, 1),
    "OR":  (0, 1),
    "AND": (0, 1),
    "CMP": (0, 1),
    
    "INC": (0,),
    "DEC": (0,),

    "SHR": (0,),
    "SHL": (0,),
    "NOT": (0,),
}


# MSG mode aliases.
MSG_MODES = {
    "OUT": 0x0000,
    "IN": 0x0001,
    "INOUT": 0x0002,
}


class AssemblerError(Exception):
    pass


def error(line_number, message):
    raise AssemblerError(
        f"line {line_number}: {message}"
    )


def parse_number(token):
    """
    Supports:

        123
        -123
        0xFF
        0b101010
    """

    try:
        return int(token, 0)
    except ValueError:
        return None


def parse_register(token):
    token = token.upper()

    if not token.startswith("R"):
        return None

    try:
        number = int(token[1:])
    except ValueError:
        return None

    if not 0 <= number <= 15:
        return None

    return number


def split_operands(text):
    if not text.strip():
        return []

    return [x.strip() for x in text.split(",")]


def clean_line(line):
    # Everything after ';' is a comment.
    return line.split(";", 1)[0].strip()


def tokenize(line):
    """
    Converts:

        LDI R0, 42

    into:

        ("LDI", ["R0", "42"])
    """

    if not line:
        return None, []

    parts = line.split(None, 1)

    mnemonic = parts[0].upper()

    if len(parts) == 1:
        return mnemonic, []

    operands = split_operands(parts[1])

    return mnemonic, operands


def parse_source(source):
    """
    Returns:

        (line_number, label, mnemonic, operands)
    """

    result = []

    for line_number, raw_line in enumerate(
        source.splitlines(), 1
    ):
        line = clean_line(raw_line)

        if not line:
            continue

        label = None

        # Supports:
        #
        # start:
        #
        # and:
        #
        # start: LDI R0, 10

        if ":" in line:
            possible_label, remainder = line.split(
                ":", 1
            )

            possible_label = possible_label.strip()

            if not possible_label:
                error(
                    line_number,
                    "empty label"
                )

            label = possible_label
            line = remainder.strip()

        if not line:
            result.append(
                (line_number, label, None, [])
            )
            continue

        mnemonic, operands = tokenize(line)

        result.append(
            (
                line_number,
                label,
                mnemonic,
                operands
            )
        )

    return result


def validate_label(label, line_number):
    if not label:
        error(
            line_number,
            "empty label"
        )

    if not (
        label[0].isalpha()
        or label[0] == "_"
    ):
        error(
            line_number,
            f"invalid label '{label}'"
        )

    for c in label:
        if not (
            c.isalnum()
            or c == "_"
        ):
            error(
                line_number,
                f"invalid character in label '{label}'"
            )


def first_pass(parsed):
    labels = {}
    instructions = []

    pc = 0
    entry_point = 0

    for line_number, label, mnemonic, operands in parsed:

        if label:
            validate_label(
                label,
                line_number
            )

            if label in labels:
                error(
                    line_number,
                    f"duplicate label '{label}'"
                )

            labels[label] = pc

        if mnemonic is None:
            continue

        if mnemonic == ".ENTRY":
            if len(operands) != 1:
                error(
                    line_number,
                    ".entry requires one argument"
                )

            entry_point = operands[0]
            continue

        if mnemonic not in OPCODES:
            error(
                line_number,
                f"unknown instruction '{mnemonic}'"
            )

        expected = OPERAND_COUNT[mnemonic]

        if len(operands) != expected:
            error(
                line_number,
                f"{mnemonic} expects "
                f"{expected} operand(s), "
                f"got {len(operands)}"
            )

        instructions.append(
            (
                line_number,
                mnemonic,
                operands
            )
        )

        pc += 1

        if pc > MAX_INSTRUCTIONS:
            error(
                line_number,
                "program is too large"
            )

    return (
        labels,
        instructions,
        entry_point
    )


def resolve_value(
    token,
    labels,
    line_number
):
    number = parse_number(token)

    if number is not None:
        return number

    if token in labels:
        return labels[token]

    error(
        line_number,
        f"unknown value or label '{token}'"
    )


def check_u16(
    value,
    token,
    line_number
):
    if not 0 <= value <= 0xFFFF:
        error(
            line_number,
            f"value '{token}' "
            f"does not fit in 16 bits"
        )


def assemble_instruction(
    line_number,
    mnemonic,
    operands,
    labels
):
    opcode = OPCODES[mnemonic]

    arg1 = 0
    arg2 = 0

    # -------------------------
    # Register instructions
    # -------------------------

    if mnemonic in REGISTER_INSTRUCTIONS:

        register_positions = (
            REGISTER_INSTRUCTIONS[mnemonic]
        )

        for position in register_positions:

            reg = parse_register(
                operands[position]
            )

            if reg is None:
                error(
                    line_number,
                    f"expected register, "
                    f"got '{operands[position]}'"
                )

            if position == 0:
                arg1 = reg
            else:
                arg2 = reg

    # -------------------------
    # JMP / JZ
    # -------------------------

    elif mnemonic in ("JMP", "JZ"):

        arg1 = resolve_value(
            operands[0],
            labels,
            line_number
        )

        check_u16(
            arg1,
            operands[0],
            line_number
        )

    # -------------------------
    # LDI
    # -------------------------

    elif mnemonic == "LDI":

        reg = parse_register(
            operands[0]
        )

        if reg is None:
            error(
                line_number,
                f"expected register, "
                f"got '{operands[0]}'"
            )

        value = resolve_value(
            operands[1],
            labels,
            line_number
        )

        check_u16(
            value,
            operands[1],
            line_number
        )

        arg1 = reg
        arg2 = value

    # -------------------------
    # MLDI
    # -------------------------

    elif mnemonic == "MLDI":

        address = resolve_value(
            operands[0],
            labels,
            line_number
        )

        value = resolve_value(
            operands[1],
            labels,
            line_number
        )

        check_u16(
            address,
            operands[0],
            line_number
        )

        check_u16(
            value,
            operands[1],
            line_number
        )

        arg1 = address
        arg2 = value

    # -------------------------
    # MW
    # -------------------------

    elif mnemonic == "MW":

        address = resolve_value(
            operands[0],
            labels,
            line_number
        )

        reg = parse_register(
            operands[1]
        )

        if reg is None:
            error(
                line_number,
                f"expected register, "
                f"got '{operands[1]}'"
            )

        check_u16(
            address,
            operands[0],
            line_number
        )

        arg1 = address
        arg2 = reg

    # -------------------------
    # MR
    # -------------------------

    elif mnemonic == "MR":

        reg = parse_register(
            operands[0]
        )

        if reg is None:
            error(
                line_number,
                f"expected register, "
                f"got '{operands[0]}'"
            )

        address = resolve_value(
            operands[1],
            labels,
            line_number
        )

        check_u16(
            address,
            operands[1],
            line_number
        )

        arg1 = reg
        arg2 = address

    # -------------------------
    # MSG
    # -------------------------

    elif mnemonic == "MSG":

        mode_token = operands[0]

        # First try named modes:
        #
        # OUT
        # IN
        # INOUT

        mode_name = mode_token.upper()

        if mode_name in MSG_MODES:
            mode = MSG_MODES[mode_name]
        else:
            mode = resolve_value(
                mode_token,
                labels,
                line_number
            )

        if not 0 <= mode <= 2:
            error(
                line_number,
                f"invalid MSG mode '{mode_token}' "
                f"(expected 0, 1, or 2)"
            )

        reg = parse_register(
            operands[1]
        )

        if reg is None:
            error(
                line_number,
                f"expected register, "
                f"got '{operands[1]}'"
            )

        arg1 = mode
        arg2 = reg

    # -------------------------
    # NOP / HLT
    # -------------------------

    else:
        pass

    # C++ struct:
    #
    # struct Instruction {
    #     uint8_t opcode;
    #     uint8_t __pad;
    #     uint16_t arg1;
    #     uint16_t arg2;
    # };
    #
    # < = little endian
    # B = uint8_t
    # B = uint8_t
    # H = uint16_t
    # H = uint16_t

    return struct.pack(
        "<BBHH",
        opcode,
        0,
        arg1,
        arg2
    )


def assemble(source):
    parsed = parse_source(source)

    (
        labels,
        instructions,
        entry
    ) = first_pass(parsed)

    # Resolve .entry after all labels
    # have been collected.

    if isinstance(entry, str):
        entry_point = resolve_value(
            entry,
            labels,
            0
        )
    else:
        entry_point = entry

    if not 0 <= entry_point < MAX_INSTRUCTIONS:
        raise AssemblerError(
            f"entry point {entry_point} "
            f"is out of range"
        )

    output = bytearray()

    # Program header.
    output += struct.pack(
        "<HH",
        MAGIC,
        entry_point
    )

    # Instructions.
    for (
        line_number,
        mnemonic,
        operands
    ) in instructions:

        output += assemble_instruction(
            line_number,
            mnemonic,
            operands,
            labels
        )

    return bytes(output)


def main():
    parser = argparse.ArgumentParser(
        description="Assembler for CEMU"
    )

    parser.add_argument(
        "input",
        help="assembly source file"
    )

    parser.add_argument(
        "-o",
        "--output",
        required=True,
        help="output binary"
    )

    args = parser.parse_args()

    try:
        with open(
            args.input,
            "r",
            encoding="utf-8"
        ) as f:
            source = f.read()

        binary = assemble(source)

        with open(
            args.output,
            "wb"
        ) as f:
            f.write(binary)

        instruction_count = (
            len(binary) - 4
        ) // INSTRUCTION_SIZE

        print(
            f"Assembled {instruction_count} "
            f"instruction(s) -> {args.output}"
        )

    except AssemblerError as e:
        print(
            f"Assembler error: {e}",
            file=sys.stderr
        )
        sys.exit(1)

    except OSError as e:
        print(
            f"File error: {e}",
            file=sys.stderr
        )
        sys.exit(1)


if __name__ == "__main__":
    main()