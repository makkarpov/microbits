package microbits.usbd.core.descriptor.structs;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.ToString;
import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.descriptor.StringReference;
import microbits.usbd.api.descriptor.runtime.DescriptorClass;
import microbits.usbd.api.descriptor.runtime.DescriptorField;
import microbits.usbd.api.descriptor.runtime.FieldType;
import microbits.usbd.core.descriptor.FieldPrinters;

@AllArgsConstructor @ToString
@Builder(toBuilder = true)
@SuppressWarnings("ClassCanBeRecord")
@DescriptorClass(type = Descriptor.TYPE_CONFIGURATION, typeName = "Configuration")
public class ConfigurationDescriptor implements Descriptor {
    public static final int LENGTH                  = 9;

    /** Reserved attribute which must be always set to one per USB specification */
    public static final int ATTR_RESERVED_ONE       = 0x80;

    /** Device is self-powered */
    public static final int ATTR_SELF_POWERED       = 0x40;

    /** Device has remote wakeup capability */
    public static final int ATTR_REMOTE_WAKEUP      = 0x20;

    /** {@code uint16_t wTotalLength} - total length of the descriptor, including all related descriptors */
    @DescriptorField(FieldType.UINT16)
    public final int wTotalLength;

    /** {@code uint8_t bNumInterfaces} - number of interfaces supported by configuration */
    @DescriptorField(FieldType.UINT8)
    public final int bNumInterfaces;

    /** {@code uint8_t bConfigurationValue} - value to use when selecting this configuration */
    @DescriptorField(FieldType.UINT8)
    public final int bConfigurationValue;

    /** {@code iConfiguration} - index of string descriptor describing this configuration */
    @DescriptorField(FieldType.UINT8_STR)
    public final StringReference iConfiguration;

    /** {@code bmAttributes} - bitmask of endpoint attributes */
    @DescriptorField(value = FieldType.UINT8_HEX, printer = FieldPrinters.ConfigAttributesPrinter.class)
    public final int bmAttributes;

    /** {@code bMaxPower} - maximum power consumption from the bus in 2mA units */
    @DescriptorField(value = FieldType.UINT8, printer = FieldPrinters.ConfigMaxPowerPrinter.class)
    public final int bMaxPower;
}
