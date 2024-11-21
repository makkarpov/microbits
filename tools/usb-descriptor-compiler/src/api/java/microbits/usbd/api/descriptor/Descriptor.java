package microbits.usbd.api.descriptor;

public interface Descriptor {
    int TYPE_DEVICE                        = 0x01;
    int TYPE_CONFIGURATION                 = 0x02;
    int TYPE_INTERFACE                     = 0x04;
    int TYPE_INTERFACE_ASSOCIATION         = 0x0B;

    /**
     * Serialize tail of the descriptor if descriptor format requires customized serialization. Serialized data will be
     * appended to the end of standard fields annotated with {@code @DescriptorField} annotation.
     *
     * <p>Returning {@code null} is interpreted as returning empty array.</p>
     */
    default byte[] serialize() {
        return null;
    }

    /**
     * Pretty-print tail of the descriptor if descriptor format requires customized serialization.
     *
     * <p>Use {@code '\t'} as a field-value separator.</p>
     *
     * <p>Returning {@code null} is interpreted as returning empty string.</p>>
     */
    default String prettyPrint() {
        return null;
    }
}
