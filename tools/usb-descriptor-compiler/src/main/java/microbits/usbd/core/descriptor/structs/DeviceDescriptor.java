package microbits.usbd.core.descriptor.structs;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.ToString;
import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.descriptor.StringReference;
import microbits.usbd.api.descriptor.runtime.DescriptorClass;
import microbits.usbd.api.descriptor.runtime.DescriptorField;
import microbits.usbd.api.descriptor.runtime.FieldType;

@AllArgsConstructor @ToString
@Builder(toBuilder = true)
@SuppressWarnings("ClassCanBeRecord")
@DescriptorClass(type = Descriptor.TYPE_DEVICE, typeName = "Device")
public class DeviceDescriptor implements Descriptor {
    /**
     * {@code uint16_t bcdUSB} - USB specification release number in binary-coded decimal as {@code 0xJJMN}, where
     * {@code JJ} is a major version number, {@code M} is a minor version number and {@code N} is a patch revision.
     * For example, value of {@code 2.00} would be represented as {@code 0x0200}.
     */
    @DescriptorField(FieldType.UINT16_BCD)
    public final int bcdUSB;

    /** {@code uint8_t bDeviceClass} - device class code */
    @DescriptorField(FieldType.UINT8_HEX)
    public final int bDeviceClass;

    /** {@code uint8_t bDeviceSubClass} - device subclass code */
    @DescriptorField(FieldType.UINT8_HEX)
    public final int bDeviceSubClass;

    /** {@code uint8_t bDeviceProtocol} - device protocol code */
    @DescriptorField(FieldType.UINT8_HEX)
    public final int bDeviceProtocol;

    /** {@code uint8_t bMaxPacketSize0} - maximum packet size for control endpoint */
    @DescriptorField(FieldType.UINT8)
    public final int bMaxPacketSize0;

    /** {@code uint16_t idVendor} - vendor identifier */
    @DescriptorField(FieldType.UINT16_HEX)
    public final int idVendor;

    /** {@code uint16_t idProduct} - product identifier */
    @DescriptorField(FieldType.UINT16_HEX)
    public final int idProduct;

    /** {@code uint16_t bcdDevice} - device version in binary-coded decimal */
    @DescriptorField(FieldType.UINT16_BCD)
    public final int bcdDevice;

    /** {@code uint8_t iManufacturer} - index of string descriptor with manufacturer name */
    @DescriptorField(FieldType.UINT8_STR)
    public final StringReference iManufacturer;

    /** {@code uint8_t iProduct} - index of string descriptor with product name */
    @DescriptorField(FieldType.UINT8_STR)
    public final StringReference iProduct;

    /** {@code uint8_t iSerialNumber} - index of string descriptor with device's serial number */
    @DescriptorField(FieldType.UINT8_STR)
    public final StringReference iSerialNumber;

    /** {@code uint8_t bNumConfigurations} - number of configurations supported by device */
    @DescriptorField(FieldType.UINT8)
    public final int bNumConfigurations;
}
