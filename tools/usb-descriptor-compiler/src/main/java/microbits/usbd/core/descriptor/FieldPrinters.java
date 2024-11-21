package microbits.usbd.core.descriptor;

import microbits.usbd.api.descriptor.Descriptor;
import microbits.usbd.api.descriptor.runtime.FieldPrinter;
import microbits.usbd.api.endpoint.EndpointType;
import microbits.usbd.api.target.LinkSpeed;
import microbits.usbd.core.descriptor.structs.ConfigurationDescriptor;
import microbits.usbd.core.descriptor.structs.EndpointDescriptor;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

import static microbits.usbd.core.descriptor.structs.EndpointDescriptor.*;

public class FieldPrinters {
    public final static class ConfigAttributesPrinter implements FieldPrinter {
        @Override
        public String printField(LinkSpeed speed, Descriptor descriptor, int value) {
            List<String> flags = new ArrayList<>();

            if ((value & ConfigurationDescriptor.ATTR_SELF_POWERED) != 0) {
                flags.add("self-powered");
            }

            if ((value & ConfigurationDescriptor.ATTR_REMOTE_WAKEUP) != 0) {
                flags.add("remote wakeup");
            }

            if (!flags.isEmpty()) {
                String flagsStr = String.join(", ", flags);
                return String.format("0x%02X\t%s", value, flagsStr);
            } else {
                return String.format("0x%02X", value);
            }
        }
    }

    public final static class ConfigMaxPowerPrinter implements FieldPrinter {
        @Override
        public String printField(LinkSpeed speed, Descriptor descriptor, int value) {
            return String.format("%d\t%d mA", value, value * 2);
        }
    }

    public final static class EndpointAddressPrinter implements FieldPrinter {
        @Override
        public String printField(LinkSpeed speed, Descriptor descriptor, int value) {
            String dir = (value & DIR_IN) != 0 ? "IN" : "OUT";
            return String.format("0x%02X\t%-3s %d", value, dir, value & ~DIR_IN);
        }
    }

    public final static class EndpointAttributesPrinter implements FieldPrinter {
        private static final String[] SYNCS = { "none", "async", "adaptive", "sync" };
        private static final String[] USAGES = { "data", "feedback", "implicit fb data" };

        @Override
        public String printField(LinkSpeed speed, Descriptor descriptor, int value) {
            EndpointType type = EndpointType.values()[value & ATTR_TYPE_MASK];
            String extra = "";

            if (type == EndpointType.ISOCHRONOUS) {
                String sync = SYNCS[(value >> ATTR_SYNC_SHIFT) & ATTR_SYNC_MASK];
                String usage = USAGES[(value >> ATTR_USAGE_SHIFT) & ATTR_USAGE_MASK];
                extra = String.format(", %s, %s", sync, usage);
            }

            return String.format("0x%02X\t%s%s", value, type, extra);
        }
    }

    public static final class EndpointPacketPrinter implements FieldPrinter {
        @Override
        public String printField(LinkSpeed speed, Descriptor descriptor, int value) {
            int bmAttributes = ((EndpointDescriptor) descriptor).bmAttributes;
            EndpointType type = EndpointType.values()[bmAttributes & ATTR_TYPE_MASK];

            if ((value >> LEN_TX_COUNT_SHIFT) == 0) {
                return String.valueOf(value);
            }

            if (speed == LinkSpeed.HIGH && (type == EndpointType.ISOCHRONOUS || type == EndpointType.INTERRUPT)) {
                int length = value & LEN_PACKET_SIZE_MASK;
                int txCount = 1 + (value >> LEN_TX_COUNT_SHIFT) & LEN_TX_COUNT_MASK;
                return String.format("0x%04X\t%d, %d tx per microframe", value, length, txCount);
            } else {
                return String.valueOf(value);
            }
        }
    }

    public static final class EndpointIntervalPrinter implements FieldPrinter {
        @Override
        public String printField(LinkSpeed speed, Descriptor descriptor, int value) {
            int bmAttributes = ((EndpointDescriptor) descriptor).bmAttributes;
            EndpointType type = EndpointType.values()[bmAttributes & ATTR_TYPE_MASK];

            int interval; // in time base units
            if (type == EndpointType.INTERRUPT && speed == LinkSpeed.FULL) interval = value;
            else if (type == EndpointType.INTERRUPT || type == EndpointType.ISOCHRONOUS) interval = 1 << (value - 1);
            else interval = value;

            int intervalUs = interval * (speed == LinkSpeed.HIGH ? 125 : 1000); // us

            String intervalStr;
            if ((intervalUs % 1000) == 0) intervalStr = String.format("%d ms", intervalUs / 1000);
            else intervalStr = String.format("%d us", intervalUs);

            return String.format("%d\t%s", value, intervalStr);
        }
    }
}
