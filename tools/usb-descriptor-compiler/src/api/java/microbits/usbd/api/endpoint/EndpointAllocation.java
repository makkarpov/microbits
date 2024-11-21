package microbits.usbd.api.endpoint;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.ToString;
import microbits.usbd.api.plugin.FunctionInstance;
import microbits.usbd.api.util.InternalApi;

import javax.annotation.Nullable;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Objects;

/**
 * Final endpoint allocation computed by the compiler.
 */
@SuppressWarnings("ClassCanBeRecord")
public final class EndpointAllocation {
    /** Description of physical IN endpoints */
    public final List<PhysicalEndpoint> inEndpoints;

    /** Description of physical OUT endpoints */
    public final List<PhysicalEndpoint> outEndpoints;

    /** Description of logical endpoints of individual functions */
    public final List<Function> functions;

    /** Merged array of IN and OUT data endpoints */
    public final List<PhysicalEndpoint> dataEndpoints;

    @InternalApi
    public EndpointAllocation(List<PhysicalEndpoint> inEndpoints, List<PhysicalEndpoint> outEndpoints,
                              List<Function> functions)
    {
        this.inEndpoints = inEndpoints;
        this.outEndpoints = outEndpoints;
        this.functions = functions;

        List<PhysicalEndpoint> data = new ArrayList<>();

        for (PhysicalEndpoint in: inEndpoints) {
            if (in.active && in.func != null) {
                data.add(in);
            }
        }

        for (PhysicalEndpoint out: outEndpoints) {
            if (out.active && out.func != null) {
                data.add(out);
            }
        }

        this.dataEndpoints = List.copyOf(data);
    }

    /** @return Endpoint array for specified direction */
    public List<PhysicalEndpoint> endpoints(EndpointDirection dir) {
        return dir == EndpointDirection.IN ? inEndpoints : outEndpoints;
    }

    /** @return Number of first inactive IN endpoint such that all IN endpoints after it are inactive */
    public int numInEndpoints() { return numEndpoints(inEndpoints); }

    /** @return Number of first inactive OUT endpoint such that all OUT endpoints after it are inactive */
    public int numOutEndpoints() { return numEndpoints(outEndpoints); }

    public int maxFuncEndpoints() {
        return functions.stream()
                .mapToInt(f -> f.endpoints.size()).max()
                .orElse(0);
    }

    @AllArgsConstructor
    @Builder @ToString
    public static class PhysicalEndpoint {
        /** Whether endpoint is active */
        public final boolean active;

        /** Endpoint number (without direction bit) */
        public final int number;

        /** Endpoint type */
        public final EndpointType type;

        /** Endpoint direction, replicated as field for convenience */
        public final EndpointDirection direction;

        /** Maximum allowed packet */
        public final int maxPacket;

        /** Number of buffers to allocate */
        public final int numBuffers;

        /** Assigned function instance */
        @Nullable
        public final FunctionInstance func;

        /** Function endpoint definition of assigned endpoint */
        @Nullable
        public final EndpointDefinition defn;

        void printString(StringBuilder r) {
            r.append("* Number:      ").append(number).append('\n');
            r.append("  Direction:   ").append(direction).append('\n');
            r.append("  Type:        ").append(type).append('\n');
            r.append("  Max packet:  ").append(maxPacket).append('\n');
            r.append("  Num buffers: ").append(numBuffers).append('\n');

            if (func != null) {
                Objects.requireNonNull(defn);
                r.append("  Mapping:     ").append(func.name()).append('.').append(defn.name).append('\n');
            }

            r.append('\n');
        }
    }

    @AllArgsConstructor
    @Builder @ToString
    public static class Function {
        /** Function instance */
        public final FunctionInstance func;

        /**
         * Logical-to-physical endpoint mapping for a function.
         * Skipped logical addresses are represented with unmapped endpoints.
         */
        public final List<FunctionEp> endpoints;

        void printString(StringBuilder r) {
            r.append("* Function:  ").append(func.name()).append('\n');
            r.append("  Endpoints: ");

            for (int i = 0; i < endpoints.size(); i++) {
                if (i != 0) {
                    r.append(", ");
                }

                FunctionEp ep = endpoints.get(i);
                if (!ep.mapped) r.append("--");
                else {
                    r.append(ep.direction.name());
                    r.append(' ');
                    r.append(ep.number);
                }
            }

            r.append("\n\n");
        }
    }

    @AllArgsConstructor
    @Builder @ToString
    public static class FunctionEp {
        /** Whether this logical endpoint is mapped anywhere */
        public final boolean mapped;

        /** Direction of mapped endpoint */
        public final EndpointDirection direction;

        /** Physical number of mapped endpoint (without a direction bit) */
        public final int number;

        /** Static unmapped value */
        public static final FunctionEp UNMAPPED = new FunctionEp(false, null, 0);
    }

    private int numEndpoints(List<PhysicalEndpoint> ep) {
        return ep.stream()
                .filter(e -> e.active)
                .map(e -> e.number)
                .max(Comparator.naturalOrder())
                .map(i -> i + 1).orElse(0);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("Physical IN endpoints:\n");
        for (PhysicalEndpoint ep: inEndpoints) {
            if (!ep.active) {
                continue;
            }

            ep.printString(sb);
        }

        sb.append("Physical OUT endpoints:\n");
        for (PhysicalEndpoint ep: outEndpoints) {
            if (!ep.active) {
                continue;
            }

            ep.printString(sb);
        }

        sb.append("Functions:\n");
        for (Function fn: functions) {
            fn.printString(sb);
        }

        return sb.toString();
    }
}
