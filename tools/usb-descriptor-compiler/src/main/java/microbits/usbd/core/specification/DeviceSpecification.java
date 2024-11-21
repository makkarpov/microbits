package microbits.usbd.core.specification;

import javax.annotation.Nullable;
import java.util.Objects;
import java.util.Set;

public class DeviceSpecification {
    public static final Set<String> RUNTIME_SERIAL_CONSTANTS = Set.of("runtime", "dynamic");

    /** USB vendor identifier value */
    public final int vendorId;

    /** USB product identifier value */
    public final int productId;

    /** USB manufacturer string descriptor */
    @Nullable
    public final String manufacturer;

    /** USB product string descriptor */
    @Nullable
    public final String product;

    /** USB serial string descriptor */
    @Nullable
    public final String serial;

    /** Whether serial number should be provided at run time */
    public final boolean runtimeSerial;

    /** Whether device is self-powered */
    public final boolean selfPowered;

    /** Maximum device power in milli amperes */
    public final int maxPower;

    /** Whether device has remove wakeup capability */
    public final boolean remoteWakeup;

    DeviceSpecification(ParsedObject data) {
        vendorId = getIdentifier(data, "vendorId");
        productId = getIdentifier(data, "productId");
        manufacturer = data.stringOpt("manufacturer");
        product = data.stringOpt("product");
        serial = data.stringOpt("serial");

        if (data.has("runtimeSerial")) {
            runtimeSerial = data.bool("runtimeSerial");
        } else {
            runtimeSerial = serial != null && RUNTIME_SERIAL_CONSTANTS.contains(serial.toLowerCase());
        }

        selfPowered = Objects.requireNonNullElse(data.boolOpt("selfPowered"), false);
        maxPower = Objects.requireNonNullElse(data.integerOpt("maxPower"), 100);
        remoteWakeup = Objects.requireNonNullElse(data.boolOpt("remoteWakeup"), false);
    }

    @Override
    public String toString() {
        return "DeviceSpecification{" +
                "vendorId=" + vendorId +
                ", productId=" + productId +
                ", manufacturer='" + manufacturer + '\'' +
                ", product='" + product + '\'' +
                ", serial='" + serial + '\'' +
                '}';
    }

    private static int getIdentifier(ParsedObject obj, String key) {
        int value = obj.integer(key);
        if (value < 0 || value > 0xFFFF) {
            throw new IllegalArgumentException(String.format(
                    "USB identifier '%s' is out of range: %#04x",
                    obj.joinPath(key), value
            ));
        }

        return value;
    }
}
