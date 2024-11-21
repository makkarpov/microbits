package microbits.usbd.api.target;

/** Target endpoint flags representing various hardware capabilities and limitations */
public enum TargetFeature {
    /** IN and OUT endpoints with same number must be of the same type */
    IN_OUT_SAME_TYPE,

    /** Double-buffering is a dedicated hardware mode, and not just arbitrarily sized FIFO */
    DOUBLE_BUF_EXPLICIT,

    /** Double-buffered endpoints must be unidirectional */
    DOUBLE_BUF_UNIDIRECTIONAL
}
