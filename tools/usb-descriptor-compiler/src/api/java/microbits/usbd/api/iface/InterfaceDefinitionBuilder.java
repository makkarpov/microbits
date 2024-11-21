package microbits.usbd.api.iface;

public final class InterfaceDefinitionBuilder {
    String name;
    int logicalNumber = -1;

    /**
     * Set the interface name. Interface names are purely informational strings and should be unique within a function.
     *
     * @param name Interface name
     * @return Builder for chained calls
     */
    public InterfaceDefinitionBuilder name(String name) {
        this.name = name;
        return this;
    }

    /**
     * Set the logical number of the interface. Logical numbers are used internally in function implementation and must
     * be unique within a function.
     *
     * @param logicalNumber Logical function number
     * @return Builder for chained calls
     */
    public InterfaceDefinitionBuilder logicalNumber(int logicalNumber) {
        this.logicalNumber = logicalNumber;
        return this;
    }

    public InterfaceDefinition build() {
        if (name == null) {
            throw new IllegalArgumentException("'name' is not defined");
        }

        if (logicalNumber < 0) {
            throw new IllegalArgumentException("'logicalNumber' is out of range: " + logicalNumber);
        }

        return new InterfaceDefinition(this);
    }
}