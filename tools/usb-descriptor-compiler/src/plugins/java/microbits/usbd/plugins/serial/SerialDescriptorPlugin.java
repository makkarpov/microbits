package microbits.usbd.plugins.serial;

import microbits.usbd.api.plugin.CompilerPlugin;
import microbits.usbd.api.plugin.USBFunction;
import microbits.usbd.api.plugin.FunctionEnvironment;
import microbits.usbd.api.util.SpecificationEntry;

import java.util.Objects;
import java.util.Set;

public class SerialDescriptorPlugin implements CompilerPlugin {
    @Override
    public Set<String> functionTypes() {
        return Set.of("serial");
    }

    @Override
    public USBFunction createFunction(String type, SpecificationEntry spec, FunctionEnvironment env) {
        int packetSize = Objects.requireNonNullElse(spec.integerOpt("packetSize"), env.speed().maxBulkPacket);
        int packetBuffers = Objects.requireNonNullElse(spec.integerOpt("dataBuffers"), 1);

        return new SerialFunction(env, packetSize, packetBuffers);
    }

    @Override
    public String toString() {
        return "SerialPlugin";
    }
}
