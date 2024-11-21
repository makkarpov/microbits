package microbits.usbd.api.descriptor;

/** Opaque reference to a string descriptor. Resolved to actual descriptor index at serialization phase. */
public interface StringReference {
    /** @return Human-readable representation of contents */
    String repr();
}
