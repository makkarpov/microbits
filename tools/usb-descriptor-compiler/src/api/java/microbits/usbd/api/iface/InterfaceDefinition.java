package microbits.usbd.api.iface;

public final class InterfaceDefinition {
    /** Name of the interface. Names should be unique within a function. */
    public final String name;

    /** Logical interface number, starting from zero. Interface numbers must be unique within a function. */
    public final int logicalNumber;

    /** Create new builder for interface definition */
    public static InterfaceDefinitionBuilder builder() {
        return new InterfaceDefinitionBuilder();
    }

    InterfaceDefinition(InterfaceDefinitionBuilder builder) {
        this.name = builder.name;
        this.logicalNumber = builder.logicalNumber;
    }
}
