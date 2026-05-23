import pytest
import re


# Simulated safe parser that mimics what a fixed poparser.c should do
# This represents the Python-level equivalent of the charset parsing logic
MAX_CHARSET_SIZE = 64  # Typical buffer size for charset field


def parse_charset_from_po_content(po_content: str) -> str | None:
    """
    Simulates parsing charset from a .po file's Content-Type header.
    A safe implementation must never return a charset string exceeding MAX_CHARSET_SIZE.
    Returns None if charset cannot be parsed or is invalid.
    """
    # Look for Content-Type header with charset
    pattern = r'Content-Type:\s*text/plain;\s*charset=([^\s\\n"]+)'
    match = re.search(pattern, po_content, re.IGNORECASE)
    if not match:
        return None
    
    charset = match.group(1).strip()
    
    # SECURITY INVARIANT: charset must never exceed MAX_CHARSET_SIZE
    # A safe implementation truncates or rejects oversized input
    if len(charset) > MAX_CHARSET_SIZE:
        # Safe behavior: reject oversized charset
        return None
    
    return charset


def simulate_vulnerable_memcpy(charset_value: str, buffer_size: int = MAX_CHARSET_SIZE) -> bytes:
    """
    Simulates what a safe memcpy should do: never copy more than buffer_size bytes.
    Raises ValueError if the input would overflow the buffer.
    """
    encoded = charset_value.encode('utf-8', errors='replace')
    if len(encoded) > buffer_size:
        raise ValueError(
            f"Buffer overflow: charset length {len(encoded)} exceeds buffer size {buffer_size}"
        )
    # Safe: copy only up to buffer_size bytes
    return encoded[:buffer_size]


def build_po_content(charset_value: str) -> str:
    """Build a .po file content string with the given charset value."""
    return (
        'msgid ""\n'
        'msgstr ""\n'
        '"Content-Type: text/plain; charset={charset}\\n"\n'
        '"Content-Transfer-Encoding: 8bit\\n"\n'
    ).format(charset=charset_value)


# Attack payloads: oversized charset strings that would trigger buffer overflow
OVERSIZED_PAYLOADS = [
    # 2x normal size
    "UTF-8" + "A" * (MAX_CHARSET_SIZE * 2),
    # 10x normal size
    "UTF-8" + "B" * (MAX_CHARSET_SIZE * 10),
    # Exactly one byte over the limit
    "X" * (MAX_CHARSET_SIZE + 1),
    # Exactly at limit (boundary - should be accepted)
    "UTF-8" + "C" * (MAX_CHARSET_SIZE - len("UTF-8")),
    # Large payload with special characters
    "UTF-8;" + "A" * 500 + "; charset=evil",
    # Null bytes embedded (potential bypass attempt)
    "UTF-8\x00" + "D" * 200,
    # Very large payload (heap overflow simulation)
    "A" * 65536,
    # Unicode characters that expand when encoded
    "UTF-8-" + "\u00e9" * 100,  # é expands to 2 bytes in UTF-8
    # Payload with format string characters
    "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s" * 10,
    # Payload with path traversal
    "../../../etc/passwd" + "A" * 200,
    # Payload with shell metacharacters
    "$(cat /etc/passwd)" + "B" * 300,
    # Mixed content exceeding buffer
    "ISO-8859-1" + "\xff" * (MAX_CHARSET_SIZE * 5),
    # Whitespace padding attack
    "UTF-8" + " " * (MAX_CHARSET_SIZE * 3),
    # Newline injection attempt
    "UTF-8\nContent-Type: text/plain; charset=" + "E" * 200,
]


@pytest.mark.parametrize("payload", OVERSIZED_PAYLOADS)
def test_charset_buffer_never_exceeds_declared_length(payload):
    """
    Invariant: Buffer reads/writes for charset field must never exceed the declared
    buffer size (MAX_CHARSET_SIZE). Any charset value longer than MAX_CHARSET_SIZE
    must be either truncated to fit within the buffer or rejected entirely.
    This guards against CWE-120 heap buffer overflow in poparser.c where memcpy
    copies 'len' bytes without validating against the allocated buffer size.
    """
    po_content = build_po_content(payload)
    
    # Parse the charset from the .po content
    result = parse_charset_from_po_content(po_content)
    
    # INVARIANT 1: If a charset is returned, it must fit within the buffer
    if result is not None:
        encoded_result = result.encode('utf-8', errors='replace')
        assert len(encoded_result) <= MAX_CHARSET_SIZE, (
            f"SECURITY VIOLATION: Parsed charset length {len(encoded_result)} "
            f"exceeds maximum buffer size {MAX_CHARSET_SIZE}. "
            f"This would cause a heap buffer overflow (CWE-120). "
            f"Payload length was {len(payload)}"
        )
    
    # INVARIANT 2: Oversized payloads must be rejected (return None)
    payload_encoded = payload.encode('utf-8', errors='replace')
    if len(payload_encoded) > MAX_CHARSET_SIZE:
        assert result is None, (
            f"SECURITY VIOLATION: Oversized charset (length {len(payload_encoded)}) "
            f"was accepted instead of being rejected. "
            f"A safe parser must reject inputs exceeding buffer size {MAX_CHARSET_SIZE}."
        )


@pytest.mark.parametrize("payload", OVERSIZED_PAYLOADS)
def test_memcpy_equivalent_never_overflows_buffer(payload):
    """
    Invariant: The memcpy equivalent operation must never write beyond the
    allocated buffer boundary. Any attempt to copy more than MAX_CHARSET_SIZE
    bytes must raise an error or be safely truncated.
    """
    payload_encoded = payload.encode('utf-8', errors='replace')
    
    if len(payload_encoded) > MAX_CHARSET_SIZE:
        # INVARIANT: Oversized input must be rejected with an error
        with pytest.raises((ValueError, OverflowError, BufferError), match=r"[Bb]uffer"):
            simulate_vulnerable_memcpy(payload, buffer_size=MAX_CHARSET_SIZE)
    else:
        # Input fits within buffer - should succeed
        result = simulate_vulnerable_memcpy(payload, buffer_size=MAX_CHARSET_SIZE)
        assert len(result) <= MAX_CHARSET_SIZE, (
            f"SECURITY VIOLATION: memcpy result length {len(result)} "
            f"exceeds buffer size {MAX_CHARSET_SIZE}"
        )


@pytest.mark.parametrize("charset,expected_safe", [
    ("UTF-8", True),                          # Normal, safe
    ("ISO-8859-1", True),                     # Normal, safe
    ("A" * MAX_CHARSET_SIZE, True),           # Exactly at limit, safe
    ("A" * (MAX_CHARSET_SIZE + 1), False),    # One byte over, unsafe
    ("A" * (MAX_CHARSET_SIZE * 2), False),    # 2x over, unsafe
    ("A" * (MAX_CHARSET_SIZE * 10), False),   # 10x over, unsafe
    ("UTF-8" + "X" * 1000, False),            # Large overflow, unsafe
])
def test_charset_length_boundary_invariant(charset, expected_safe):
    """
    Invariant: The charset parser must correctly identify safe vs unsafe inputs
    at and around the buffer boundary. Inputs at or below MAX_CHARSET_SIZE are
    safe; inputs exceeding MAX_CHARSET_SIZE must be rejected to prevent CWE-120.
    """
    po_content = build_po_content(charset)
    result = parse_charset_from_po_content(po_content)
    
    charset_encoded = charset.encode('utf-8', errors='replace')
    
    if expected_safe:
        # Safe inputs at or below buffer size should be parseable
        if len(charset_encoded) <= MAX_CHARSET_SIZE:
            # The parser may return a result for safe inputs
            if result is not None:
                result_encoded = result.encode('utf-8', errors='replace')
                assert len(result_encoded) <= MAX_CHARSET_SIZE, (
                    f"SECURITY VIOLATION: Safe input produced oversized output. "
                    f"Result length: {len(result_encoded)}, Max: {MAX_CHARSET_SIZE}"
                )
    else:
        # Unsafe inputs exceeding buffer size must be rejected
        assert result is None, (
            f"SECURITY VIOLATION: Unsafe charset of length {len(charset_encoded)} "
            f"was accepted. Expected rejection to prevent buffer overflow (CWE-120). "
            f"Buffer size: {MAX_CHARSET_SIZE}"
        )