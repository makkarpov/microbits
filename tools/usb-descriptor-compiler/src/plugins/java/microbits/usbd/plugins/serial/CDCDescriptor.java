package microbits.usbd.plugins.serial;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.ToString;
import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.descriptor.runtime.DescriptorClass;
import microbits.usbd.api.descriptor.runtime.DescriptorField;
import microbits.usbd.api.descriptor.runtime.FieldType;

@DescriptorClass(
        type = CDCDescriptor.CDC_DESCRIPTOR_TYPE,
        typeName = "CDC Interface Specific",
        nextField = @DescriptorClass.NextTypeField(name = "bDescriptorSubtype", type = FieldType.UINT8)
)
public interface CDCDescriptor extends Descriptor {
    int CDC_DESCRIPTOR_TYPE = 0x24;

    int CS_TYPE_HEADER      = 0x00;
    int CS_TYPE_CALL_MGMT   = 0x01;
    int CS_TYPE_ACM         = 0x02;
    int CS_TYPE_UNION       = 0x03;

    @DescriptorClass(type = CS_TYPE_HEADER, typeName = "CDC Header")
    @AllArgsConstructor @Builder @ToString
    @SuppressWarnings("ClassCanBeRecord")
    class HeaderDescriptor implements CDCDescriptor {
        @DescriptorField(FieldType.UINT16_BCD)
        public final int bcdCDC;
    }

    @DescriptorClass(type = CS_TYPE_CALL_MGMT, typeName = "CDC Call Management")
    @AllArgsConstructor @Builder @ToString
    @SuppressWarnings("ClassCanBeRecord")
    class CallManagementDescriptor implements CDCDescriptor {
        @DescriptorField(FieldType.UINT8_HEX)
        public final int bmCapabilities;

        @DescriptorField(FieldType.UINT8)
        public final int bDataInterface;
    }

    @DescriptorClass(type = CS_TYPE_ACM, typeName = "CDC ACM")
    @AllArgsConstructor @Builder @ToString
    @SuppressWarnings("ClassCanBeRecord")
    class ACMDescriptor implements CDCDescriptor {
        @DescriptorField(FieldType.UINT8_HEX)
        public final int bmCapabilities;
    }

    @DescriptorClass(type = CS_TYPE_UNION, typeName = "CDC Interface Union")
    @AllArgsConstructor @Builder @ToString
    @SuppressWarnings("ClassCanBeRecord")
    class InterfaceUnionDescriptor implements CDCDescriptor {
        @DescriptorField(FieldType.UINT8)
        public final int bMasterInterface;

        @DescriptorField(FieldType.UINT8)
        public final int bSlaveInterface;
    }
}
