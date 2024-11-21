package microbits.usbd.core.codegen;

import microbits.usbd.api.codegen.TargetCodeGenerator;
import microbits.usbd.api.codegen.TargetConfigurationData;
import microbits.usbd.api.endpoint.EndpointAllocation;
import microbits.usbd.api.target.LinkSpeed;
import microbits.usbd.core.util.PrinterUtils;

import javax.annotation.Nullable;
import java.util.List;

public class GenericTargetGenerator implements TargetCodeGenerator {
    @Override
    public String targetIdentifier() {
        return "generic";
    }

    private void generateEndpointArray(StringBuilder ret, String symbolName, EndpointAllocation endpoints) {
        ret.append("static const " + ENDPOINT_CFG_TYPE + " ").append(symbolName).append("[] {\n");

        for (int i = 0; i < endpoints.dataEndpoints.size(); i++) {
            EndpointAllocation.PhysicalEndpoint ep = endpoints.dataEndpoints.get(i);
            ret.append("  { ");

            int flags = 0;
            if (ep.numBuffers != 1) {
                if (ep.numBuffers != 2) {
                    throw new RuntimeException("Only double buffering is supported");
                }

                flags |= EP_FLAG_DOUBLE_BUF;
            }

            PrinterUtils.appendHexByte(ret, ep.number | ep.direction.directionBit);
            ret.append(", ");

            ret.append(ENDPOINT_TYPE_TYPE).append("::").append(ep.type.name());
            ret.append(", ");

            ret.append(ep.maxPacket).append(", ");

            PrinterUtils.appendHexByte(ret, flags);

            ret.append(" }");
            if (i != endpoints.dataEndpoints.size() - 1) ret.append(",");
            ret.append("\n");
        }

        ret.append("};\n\n");
    }

    private void generateFlags(StringBuilder ret, TargetConfigurationData data) {
        int flags = 0;
        if (data.speeds().contains(LinkSpeed.HIGH)) {
            flags |= CFG_FLAG_HIGH_SPEED;
        }

        ret.append("  ");
        PrinterUtils.appendHexByte(ret, flags);
        ret.append(",\n");
    }

    private void generateEndpointRefs(StringBuilder ret, String symbolName, TargetConfigurationData data) {
        List<LinkSpeed> speeds = data.speeds();

        ret.append("  { ");
        for (int i = 0; i < speeds.size(); i++) {
            EndpointAllocation eps = data.endpoints(speeds.get(i));

            if (i != 0) ret.append(", ");
            ret.append(eps.dataEndpoints.size());
        }
        ret.append(" },\n");

        ret.append("  { ");
        for (int i = 0; i < speeds.size(); i++) {
            LinkSpeed speed = speeds.get(i);

            if (i != 0) ret.append(", ");
            ret.append(endpointArraySymbol(symbolName, speed));
        }
        ret.append(" }\n");
    }

    @Nullable
    @Override
    public String generateConfiguration(String symbolName, TargetConfigurationData data) {
        StringBuilder ret = new StringBuilder();

        for (LinkSpeed speed: data.speeds()) {
            generateEndpointArray(ret, endpointArraySymbol(symbolName, speed), data.endpoints(speed));
        }

        ret.append("static const ").append(GENERIC_DATA_TYPE).append(' ');
        ret.append(symbolName).append(" {\n");

        generateFlags(ret, data);
        generateEndpointRefs(ret, symbolName, data);

        ret.append("};\n");

        return ret.toString();
    }

    private static String endpointArraySymbol(String baseSymbol, LinkSpeed speed) {
        return String.format(ENDPOINT_ARRAY_SYM, baseSymbol, PrinterUtils.capitalize(speed.name()));
    }

    private static final int CFG_FLAG_HIGH_SPEED = 0x01;
    private static final int EP_FLAG_DOUBLE_BUF = 0x02;

    private static final String GENERIC_DATA_TYPE   = "::ub::usbd::config::GenericTargetData";
    private static final String ENDPOINT_CFG_TYPE   = "::ub::usbd::config::EndpointConfig";
    private static final String ENDPOINT_TYPE_TYPE  = "::ub::usbd::EndpointType";
    private static final String ENDPOINT_ARRAY_SYM = "%s_ep%s";
}
