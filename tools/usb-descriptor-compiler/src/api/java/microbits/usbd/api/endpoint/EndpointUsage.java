package microbits.usbd.api.endpoint;

/** Isochronous endpoint usage type part of bmAttributes field */
public enum EndpointUsage {
    DATA,
    FEEDBACK,
    IMPLICIT_FB_DATA,
}
