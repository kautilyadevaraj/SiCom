#pragma once
#include <QString>
#include <QByteArray>

// Pure static utility class — no state, no constructor needed.
// Call as: DataConverter::toHex(data), DataConverter::parse(str, fmt), etc.
class DataConverter {
public:
    enum class Format { Ascii = 0, Hex = 1, Binary = 2, Decimal = 3 };

    // ── Display (bytes → human-readable string) ──────────────────────────
    static QString toHex    (const QByteArray &data, bool spaced = true);
    // spaced=true  → "AA BB CC"   spaced=false → "AABBCC"

    static QString toBinary (const QByteArray &data, bool spaced = true);
    // spaced=true  → "10101010 11001100"

    static QString toDecimal(const QByteArray &data);
    // Each byte as unsigned int, space-separated → "170 204 255"

    static QString toAscii  (const QByteArray &data);
    // Printable chars as-is, non-printable replaced with '.' → "AT..OK"

    // ── Parse (user string → raw bytes for sending) ───────────────────
    static QByteArray fromHex    (const QString &input); // "AA BB" or "AABB"
    static QByteArray fromBinary (const QString &input); // "10101010 11001100"
    static QByteArray fromDecimal(const QString &input); // "170 204 255"
    static QByteArray fromAscii  (const QString &input); // plain text

    // ── Validation (call on textChanged before allowing send) ─────────
    static bool isValidHex    (const QString &input);
    static bool isValidBinary (const QString &input);
    static bool isValidDecimal(const QString &input);

    // ── Dispatch helpers (use these everywhere instead of switch-case) ─
    static QByteArray parse  (const QString  &input, Format fmt);
    static QString    display(const QByteArray &data, Format fmt);
    static bool       isValid(const QString  &input, Format fmt);
    // ASCII is always valid — isValid returns true for Format::Ascii.

    // ── Index ↔ Format (keeps combo box index in sync) ────────────────
    static Format     formatFromIndex(int index);  // 0=Ascii 1=Hex 2=Bin 3=Dec
    static QString    formatName(Format fmt);       // "ASCII", "Hex", etc.
};