package microbits.usbd.core.descriptor.structs;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.ToString;
import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.descriptor.runtime.DescriptorClass;
import microbits.usbd.api.descriptor.runtime.DescriptorField;
import microbits.usbd.api.descriptor.runtime.FieldType;
import microbits.usbd.core.descriptor.FieldPrinters;

@AllArgsConstructor @ToString
@Builder(toBuilder = true)
@SuppressWarnings("ClassCanBeRecord")
@DescriptorClass(type = EndpointDescriptor.TYPE_ENDPOINT, typeName = "Endpoint")
public class EndpointDescriptor implements Descriptor {
    public static final int TYPE_ENDPOINT   = 0x05;
    public static final int DIR_IN          = 0x80;

    public static final int ATTR_TYPE_MASK              = 0x03;
    // See EndpointType enumeration for values

    public static final int ATTR_SYNC_SHIFT             = 2;
    public static final int ATTR_SYNC_MASK              = 0x03;
    public static final int ATTR_SYNC_NONE              = 0x00;
    public static final int ATTR_SYNC_ASYNC             = 0x01;
    public static final int ATTR_SYNC_ADAPTIVE          = 0x02;
    public static final int ATTR_SYNC_SYNCHRONOUS       = 0x03;

    public static final int ATTR_USAGE_SHIFT            = 4;
    public static final int ATTR_USAGE_MASK             = 0x03;
    public static final int ATTR_USAGE_DATA             = 0x00;
    public static final int ATTR_USAGE_FEEDBACK         = 0x01;
    public static final int ATTR_USAGE_IMPLICIT_FB_DATA = 0x02;

    public static final int LEN_PACKET_SIZE_MASK        = 0x7FF;
    public static final int LEN_TX_COUNT_SHIFT          = 11;
    public static final int LEN_TX_COUNT_MASK           = 0x03;

    /** {@code uint8_t bEndpointAddress} - address of the endpoint with direction bit */
    @DescriptorField(value = FieldType.UINT8, printer = FieldPrinters.EndpointAddressPrinter.class)
    public final int bEndpointAddress;

    /** {@code uint8_t bmAttributes} - endpoint attributes bitmask */
    @DescriptorField(value = FieldType.UINT8, printer = FieldPrinters.EndpointAttributesPrinter.class)
    public final int bmAttributes;

    /** {@code uint16_t wMaxPacketSize} - endpoint packet size */
    @DescriptorField(value = FieldType.UINT16, printer = FieldPrinters.EndpointPacketPrinter.class)
    public final int wMaxPacketSize;

    /** {@code uint8_t bInterval} - endpoint polling interval */
    @DescriptorField(value = FieldType.UINT8, printer = FieldPrinters.EndpointIntervalPrinter.class)
    public final int bInterval;
}
