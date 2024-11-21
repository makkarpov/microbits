package microbits.usbd.core.state;

import microbits.usbd.api.endpoint.EndpointAllocation;
import microbits.usbd.api.endpoint.EndpointDefinition;
import microbits.usbd.api.endpoint.EndpointDirection;
import microbits.usbd.api.endpoint.EndpointType;
import microbits.usbd.api.plugin.FunctionInstance;
import microbits.usbd.api.target.TargetFeature;
import microbits.usbd.api.target.TargetProperties;
import microbits.usbd.core.specification.ControlPipeSpecification;

import java.util.*;
import java.util.stream.IntStream;

public class EndpointAllocator {
    private final TargetProperties target;

    private final AllocState[] in;
    private final AllocState[] out;

    // func_id -> logical_addr -> ep
    private final Map<Integer, Map<Integer, EndpointAllocation.FunctionEp>> functions;

    // References to in and out arrays matched with EndpointDirection.ordinal() values: [0] = in, [1] = out
    private final AllocState[][] directions;

    public EndpointAllocator(TargetProperties target) {
        this.target = target;

        this.in = createEndpoints(target.inEndpoints);
        this.out = createEndpoints(target.outEndpoints);

        this.directions = new AllocState[][] { in, out };
        this.functions = new HashMap<>();
    }

    private AllocState[] createEndpoints(int count) {
        AllocState[] r = new AllocState[count];

        for (int i = 0; i < count; i++) {
            r[i] = new AllocState();
            r[i].number = i;
        }

        r[0].status = Status.CONTROL;
        r[0].type = EndpointType.CONTROL;

        return r;
    }

    public void addEndpoint(FunctionState func, EndpointDefinition defn) {
        AllocState[] states = directions[defn.direction.ordinal()];
        AllocState[] revStates = directions[1 - defn.direction.ordinal()];

        if (target.features.contains(TargetFeature.DOUBLE_BUF_EXPLICIT) && defn.numBuffers > 2) {
            throw new EndpointAllocationFailure(String.format(
                    "Target hardware %s does not allow for more than two buffers per endpoint: " +
                            "'%s.%s' has numBuffers=%d",
                    target.name, func.name, defn.name, defn.numBuffers
            ));
        }

        for (int i = 0; i < states.length; i++) {
            AllocState fwd = states[i];
            AllocState rev = i < revStates.length ? revStates[i] : null;
            boolean invalidateReverse = false;

            if (fwd.status != Status.FREE) {
                continue;
            }
            
            if (rev != null) {
                // IN_OUT_SAME_TYPE hardware constraint implies that reverse direction must be either free or
                // be a data endpoint with same type
                if (target.features.contains(TargetFeature.IN_OUT_SAME_TYPE) && rev.status != Status.FREE &&
                        (rev.status != Status.DATA || rev.type != defn.type))
                {
                    continue;
                }

                // DOUBLE_BUF_UNIDIRECTIONAL hardware constraint implies that reverse direction must be unused:
                if (target.features.contains(TargetFeature.DOUBLE_BUF_UNIDIRECTIONAL) && defn.numBuffers > 1) {
                    if (rev.status != Status.FREE) {
                        continue;
                    }

                    invalidateReverse = true;
                }
            }

            fwd.status = Status.DATA;
            fwd.type   = defn.type;
            fwd.func   = func;
            fwd.defn   = defn;

            if (invalidateReverse) {
                rev.status = Status.INVALID;
            }

            functions
                    .computeIfAbsent(func.id, k -> new HashMap<>())
                    .put(defn.logicalAddress, new EndpointAllocation.FunctionEp(true, defn.direction, i));

            return;
        }

        throw new EndpointAllocationFailure();
    }

    private EndpointAllocation.PhysicalEndpoint transformEndpoint(AllocState st, EndpointDirection dir,
                                                                  ControlPipeSpecification control)
    {
        if (st.status != Status.CONTROL && st.status != Status.DATA) {
            return EndpointAllocation.PhysicalEndpoint.builder()
                    .number(st.number)
                    .direction(dir)
                    .build();
        }

        return EndpointAllocation.PhysicalEndpoint.builder()
                .active(true)
                .number(st.number)
                .type(st.type)
                .direction(dir)
                .maxPacket(st.status == Status.CONTROL ? control.maxPacket : st.defn.packetSize)
                .numBuffers(st.status == Status.CONTROL ? 1 : st.defn.numBuffers)
                .func(st.func)
                .defn(st.defn)
                .build();
    }

    private EndpointAllocation.Function transformFunction(FunctionInstance func) {
        Map<Integer, EndpointAllocation.FunctionEp> rawMap = functions.getOrDefault(func.identifier(), Map.of());
        int numLogicalEndpoints = rawMap.keySet().stream().max(Comparator.naturalOrder()).orElse(-1) + 1;

        List<EndpointAllocation.FunctionEp> endpoints = IntStream.range(0, numLogicalEndpoints)
                .mapToObj(i -> rawMap.getOrDefault(i, EndpointAllocation.FunctionEp.UNMAPPED))
                .toList();

        return EndpointAllocation.Function.builder()
                .func(func)
                .endpoints(endpoints)
                .build();
    }

    public EndpointAllocation finish(ControlPipeSpecification control, List<? extends FunctionInstance> functions) {
        List<EndpointAllocation.PhysicalEndpoint> in = Arrays.stream(this.in)
                .map(st -> transformEndpoint(st, EndpointDirection.IN, control))
                .toList();

        List<EndpointAllocation.PhysicalEndpoint> out = Arrays.stream(this.out)
                .map(st -> transformEndpoint(st, EndpointDirection.OUT, control))
                .toList();

        List<EndpointAllocation.Function> funcs = functions.stream()
                .sorted(Comparator.comparingInt(FunctionInstance::identifier))
                .map(this::transformFunction)
                .toList();

        return new EndpointAllocation(in, out, funcs);
    }

    private enum Status {
        /** Endpoint is reserved for control transfers */
        CONTROL,

        /** Endpoint cannot be used due to hardware constraints */
        INVALID,

        /** Endpoint is allocated for data transfers */
        DATA,

        /** Endpoint is free to be used */
        FREE
    }

    private static class AllocState {
        /** Endpoint number (address without direction bit) */
        int                 number;

        /** Endpoint status */
        Status              status = Status.FREE;

        /** Endpoint type */
        EndpointType        type;

        /** Allocated function, if any */
        FunctionState       func;

        /** Allocated endpoint definition, if any */
        EndpointDefinition  defn;
    }

    public static class EndpointAllocationFailure extends RuntimeException {
        public EndpointAllocationFailure() {
        }

        public EndpointAllocationFailure(String message) {
            super(message);
        }
    }
}
