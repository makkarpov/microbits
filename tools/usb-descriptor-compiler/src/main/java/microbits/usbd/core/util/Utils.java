package microbits.usbd.core.util;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;

public class Utils {
    public static String formatImplementationId(String targetId) {
        try {
            byte[] dgst = MessageDigest
                    .getInstance("SHA-256")
                    .digest(targetId.getBytes(StandardCharsets.UTF_8));

            long runtimeId = ((dgst[0] & 0xFFL) << 24) | ((dgst[1] & 0xFFL) << 16) |
                    ((dgst[2] & 0xFFL) << 8) | (dgst[3] & 0xFFL);

            return String.format("0x%08X /* %s */", runtimeId, PrinterUtils.stringRepr(targetId));
        } catch (Exception e) {
            throw new RuntimeException("Failed to compute implementation ID", e);
        }
    }
}
