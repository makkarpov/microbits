package microbits.usbd.core.descriptor;

/**
 * @param fieldName   Name of the associated field
 * @param prettyValue Pretty-printed value of the field
 * @param serialized  Serialized field data
 */
public record SerializedField(String fieldName, String prettyValue, byte[] serialized) {
    /** Serialized length of the field */
    public int length() {
        return serialized.length;
    }
}
