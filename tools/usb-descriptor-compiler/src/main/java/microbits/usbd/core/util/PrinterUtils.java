package microbits.usbd.core.util;

public class PrinterUtils {
    public static char hexDigit(int x) {
        if (x < 10) return (char) ('0' + x);
        else return (char) ('A' + (x - 10));
    }

    private static void appendUnicodeEscape(StringBuilder sb, char c) {
        sb.append('u');
        sb.append(hexDigit((c >> 12) & 0xF));
        sb.append(hexDigit((c >> 8) & 0xF));
        sb.append(hexDigit((c >> 4) & 0xF));
        sb.append(hexDigit(c & 0xF));
    }

    public static void appendHexByte(StringBuilder sb, int x) {
        sb.append("0x");
        sb.append(hexDigit((x >> 4) & 0xF));
        sb.append(hexDigit(x & 0xF));
    }

    public static String stringRepr(String s) {
        StringBuilder r = new StringBuilder(2 + s.length());
        r.append('\'');

        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);

            if (c < ' ') {
                r.append('\\');
                switch (c) {
                case '\n': r.append('n'); break;
                case '\r': r.append('r'); break;
                case '\t': r.append('t'); break;
                case '\f': r.append('f'); break;
                case '\b': r.append('b'); break;
                default: appendUnicodeEscape(r, c); break;
                }
            } else if (c == '\\' || c == '\'' || c == '\"') {
                r.append('\\');
                r.append(c);
            } else {
                r.append(c);
            }
        }

        r.append('\'');
        return r.toString();
    }

    public static String capitalize(String s) {
        if (s.length() < 2) {
            return s.toUpperCase();
        }

        return Character.toUpperCase(s.charAt(0)) + s.substring(1).toLowerCase();
    }
}
