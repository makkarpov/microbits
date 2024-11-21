package microbits.usbd.api.endpoint;

/** Endpoint types enumeration, ordered in the same way as for endpoint descriptor bmAttributes field */
public enum EndpointType {
    /** Control endpoint - not used in any of the function descriptors, included for completeness */
    CONTROL,

    /** Isochronous endpoint - guaranteed bandwidth and latency, non-reliable delivery */
    ISOCHRONOUS,

    /** Bulk endpoint - best-effort bandwidth, reliable delivery */
    BULK,

    /** Interrupt endpoint - guaranteed latency, reliable delivery */
    INTERRUPT;
}
