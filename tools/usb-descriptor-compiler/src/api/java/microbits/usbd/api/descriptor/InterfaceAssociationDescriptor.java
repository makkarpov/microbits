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
@DescriptorClass(type = Descriptor.TYPE_INTERFACE_ASSOCIATION, typeName = "InterfaceAssociation")
public final class InterfaceAssociationDescriptor implements Descriptor {
    /** {@code uint8_t bFirstInterface} - number of first interface in the association, inclusive. */
    @DescriptorField(FieldType.UINT8)
    public final int bFirstInterface;

    /** {@code uint8_t bInterfaceCount} - number of associated interfaces */
    @DescriptorField(FieldType.UINT8)
    public final int bInterfaceCount;

    /** {@code uint8_t bFunctionClass} - function class identifier */
    @DescriptorField(FieldType.UINT8_HEX)
    public final int bFunctionClass;

    /** {@code uint8_t bFunctionSubClass} - function subclass identifier */
    @DescriptorField(FieldType.UINT8_HEX)
    public final int bFunctionSubclass;

    /** {@code uint8_t bFunctionProtocol} - function protocol identifier */
    @DescriptorField(FieldType.UINT8_HEX)
    public final int bFunctionProtocol;

    /** {@code uint8_t iFunction} - index of function string descriptor */
    @DescriptorField(FieldType.UINT8_STR)
    public final StringReference iFunction;
}
