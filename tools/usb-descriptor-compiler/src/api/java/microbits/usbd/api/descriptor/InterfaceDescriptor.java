package microbits.usbd.api.descriptor;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.ToString;
import microbits.usbd.api.descriptor.runtime.DescriptorClass;
import microbits.usbd.api.descriptor.runtime.DescriptorField;
import microbits.usbd.api.descriptor.runtime.FieldType;

@AllArgsConstructor @ToString
@Builder(toBuilder = true)
@SuppressWarnings("ClassCanBeRecord")
@DescriptorClass(type = Descriptor.TYPE_INTERFACE, typeName = "Interface")
public final class InterfaceDescriptor implements Descriptor {
    /** Length of the descriptor in bytes */
    public static final int LENGTH = 9;

    /** {@code uint8_t bInterfaceNumber} - number of this interface */
    @DescriptorField(FieldType.UINT8)
    public final int bInterfaceNumber;

    /** {@code uint8_t bAlternateSetting} - value to use when selecting interface alternate setting */
    @DescriptorField(FieldType.UINT8)
    public final int bAlternateSetting;

    /** {@code uint8_t bNumEndpoints} - number of data endpoints used by this interface */
    @DescriptorField(FieldType.UINT8)
    public final int bNumEndpoints;

    /** {@code uint8_t bInterfaceClass} - class code of the interface */
    @DescriptorField(FieldType.UINT8_HEX)
    public final int bInterfaceClass;

    /** {@code uint8_t bInterfaceSubClass} - subclass code of the interface */
    @DescriptorField(FieldType.UINT8_HEX)
    public final int bInterfaceSubclass;

    /** {@code uint8_t bInterfaceProtocol} - protocol code of the interface */
    @DescriptorField(FieldType.UINT8_HEX)
    public final int bInterfaceProtocol;

    /** {@code uint8_t iInterface} - index string descriptor for the interface */
    @DescriptorField(FieldType.UINT8_STR)
    public final StringReference iInterface;
}
