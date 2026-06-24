#include "dataconvertor.h"
#include <QRegularExpression>

// ── Display ──────────────────────────────────────────────────────────────────

QString DataConverter::toHex(const QByteArray &data, bool spaced) {
    // toHex() returns lowercase; toUpper() for readability ("AA" not "aa")
    QByteArray hex = data.toHex(spaced ? ' ' : '\0').toUpper();
    return QString::fromLatin1(hex);
}

QString DataConverter::toBinary(const QByteArray &data, bool spaced) {
    QStringList parts;
    for (unsigned char byte : data)
        // arg(val, width, base, fillChar) → pads to 8 digits with leading zeros
        parts << QString("%1").arg(byte, 8, 2, QChar('0'));
    return parts.join(spaced ? " " : "");
}

QString DataConverter::toDecimal(const QByteArray &data) {
    QStringList parts;
    for (unsigned char byte : data)
        parts << QString::number(byte);
    return parts.join(' ');
}

QString DataConverter::toAscii(const QByteArray &data) {
    QString result;
    result.reserve(data.size());
    for (unsigned char byte : data)
        result += (byte >= 0x20 && byte < 0x7F) ? QChar(byte) : QChar('.');
    return result;
}

// ── Parse ────────────────────────────────────────────────────────────────────

QByteArray DataConverter::fromHex(const QString &input) {
    // Strip all whitespace so "AA BB" and "AABB" both work
    QString cleaned = input;
    cleaned.remove(' ');
    return QByteArray::fromHex(cleaned.toLatin1());
}

QByteArray DataConverter::fromBinary(const QString &input) {
    QString cleaned = input;
    cleaned.remove(' ');
    QByteArray result;
    // Process 8 chars at a time — each group is one byte
    for (int i = 0; i + 8 <= cleaned.length(); i += 8) {
        bool ok;
        char byte = static_cast<char>(cleaned.mid(i, 8).toInt(&ok, 2));
        if (ok) result.append(byte);
    }
    return result;
}

QByteArray DataConverter::fromDecimal(const QString &input) {
    QByteArray result;
    const QStringList parts = input.trimmed().split(' ', Qt::SkipEmptyParts);
    for (const QString &s : parts) {
        bool ok;
        int val = s.toInt(&ok);
        if (ok && val >= 0 && val <= 255)
            result.append(static_cast<char>(val));
    }
    return result;
}

QByteArray DataConverter::fromAscii(const QString &input) {
    return input.toLatin1();
}

// ── Validation ───────────────────────────────────────────────────────────────

bool DataConverter::isValidHex(const QString &input) {
    if (input.trimmed().isEmpty()) return false;
    // Accepts: "AABB" or "AA BB CC" — hex pairs with optional single spaces
    static QRegularExpression re(
        R"(^[0-9A-Fa-f]{2}(\s[0-9A-Fa-f]{2})*$|^([0-9A-Fa-f]{2})+$)"
        );
    return re.match(input.trimmed()).hasMatch();
}

bool DataConverter::isValidBinary(const QString &input) {
    if (input.trimmed().isEmpty()) return false;
    // Accepts: "10101010" or "10101010 11001100" — 8-bit groups, optional spaces
    static QRegularExpression re(
        R"(^[01]{8}(\s[01]{8})*$|^([01]{8})+$)"
        );
    return re.match(input.trimmed()).hasMatch();
}

bool DataConverter::isValidDecimal(const QString &input) {
    if (input.trimmed().isEmpty()) return false;
    // Every space-separated token must be 0–255
    const QStringList parts = input.trimmed().split(' ', Qt::SkipEmptyParts);
    for (const QString &s : parts) {
        bool ok;
        int v = s.toInt(&ok);
        if (!ok || v < 0 || v > 255) return false;
    }
    return true;
}

// ── Dispatch ─────────────────────────────────────────────────────────────────

QByteArray DataConverter::parse(const QString &input, Format fmt) {
    switch (fmt) {
    case Format::Hex:     return fromHex(input);
    case Format::Binary:  return fromBinary(input);
    case Format::Decimal: return fromDecimal(input);
    case Format::Ascii:   // fallthrough
    default:              return fromAscii(input);
    }
}

QString DataConverter::display(const QByteArray &data, Format fmt) {
    switch (fmt) {
    case Format::Hex:     return toHex(data);
    case Format::Binary:  return toBinary(data);
    case Format::Decimal: return toDecimal(data);
    case Format::Ascii:   // fallthrough
    default:              return toAscii(data);
    }
}

bool DataConverter::isValid(const QString &input, Format fmt) {
    switch (fmt) {
    case Format::Hex:     return isValidHex(input);
    case Format::Binary:  return isValidBinary(input);
    case Format::Decimal: return isValidDecimal(input);
    case Format::Ascii:   // ASCII is always valid
    default:              return true;
    }
}

DataConverter::Format DataConverter::formatFromIndex(int index) {
    // Matches the order items are added to combo boxes in setupCombos()
    switch (index) {
    case 1:  return Format::Hex;
    case 2:  return Format::Binary;
    case 3:  return Format::Decimal;
    default: return Format::Ascii;
    }
}

QString DataConverter::formatName(Format fmt) {
    switch (fmt) {
    case Format::Hex:     return "Hex";
    case Format::Binary:  return "Binary";
    case Format::Decimal: return "Decimal";
    default:              return "ASCII";
    }
}