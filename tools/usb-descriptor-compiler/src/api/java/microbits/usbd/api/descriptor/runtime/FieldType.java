package microbits.usbd.api.descriptor.runtime;

public enum FieldType {
    /** Unsigned byte field, printed as decimal number */
    UINT8(1),

    /** Unsigned byte field, printed as hexadecimal number with leading zeros */
    UINT8_HEX(1),

    /** Unsigned byte field, referring to a string descriptor */
    UINT8_STR(1),

    /** Unsigned short field, printed as decimal number */
    UINT16(2),

    /** Unsigned short field, printed as hexadecimal number with leading zeros */
    UINT16_BCD(2),

    /** Unsigned short field, printed as hexadecimal number with BCD version decoding */
    UINT16_HEX(2);

    /** Length of the field in bytes */
    public final int length;

    FieldType(int length) {
        this.length = length;
    }
}
