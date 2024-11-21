package microbits.usbd.core.codegen;

import microbits.usbd.api.codegen.ConfigProperty;

import static microbits.usbd.api.codegen.ConfigProperty.Type.*;

public class ConfigProperties {
    public static final ConfigProperty IN_ENDPOINTS = new ConfigProperty("UB_USBD_MAX_IN_ENDPOINTS", SIZE);
    public static final ConfigProperty OUT_ENDPOINTS = new ConfigProperty("UB_USBD_MAX_OUT_ENDPOINTS", SIZE);
    public static final ConfigProperty FUNCTIONS = new ConfigProperty("UB_USBD_MAX_FUNCTIONS", SIZE);
    public static final ConfigProperty INTERFACES = new ConfigProperty("UB_USBD_MAX_INTERFACES", SIZE);
    public static final ConfigProperty FUNC_ENDPOINTS = new ConfigProperty("UB_USBD_MAX_FUNC_ENDPOINTS", SIZE);
    public static final ConfigProperty CONTROL_PACKET = new ConfigProperty("UB_USBD_MAX_CONTROL_PACKET", VALUE);
    public static final ConfigProperty RUNTIME_SERIAL_NR = new ConfigProperty("UB_USBD_RUNTIME_SERIAL_NUMBER", FLAG);
    public static final ConfigProperty HIGH_SPEED = new ConfigProperty("UB_USBD_ENABLE_HIGH_SPEED", FLAG);
}
