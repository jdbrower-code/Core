using System.Drawing;

namespace COSM_EIS
{
    /// <summary>
    /// Centralized color palette matching the original dark professional design.
    /// </summary>
    internal static class AppColors
    {
        public static readonly Color Bg      = ColorTranslator.FromHtml("#0C0F18");
        public static readonly Color Surf    = ColorTranslator.FromHtml("#111827");
        public static readonly Color Surf2   = ColorTranslator.FromHtml("#151D2E");
        public static readonly Color Surf3   = ColorTranslator.FromHtml("#1A2438");
        public static readonly Color Border  = ColorTranslator.FromHtml("#1E2D47");
        public static readonly Color Border2 = ColorTranslator.FromHtml("#243349");
        public static readonly Color Accent  = ColorTranslator.FromHtml("#2D6FFF");
        public static readonly Color Accent2 = ColorTranslator.FromHtml("#00D4AA");
        public static readonly Color Text    = ColorTranslator.FromHtml("#E2E8F5");
        public static readonly Color Muted   = ColorTranslator.FromHtml("#4A5978");
        public static readonly Color Muted2  = ColorTranslator.FromHtml("#637294");
        public static readonly Color Gold    = ColorTranslator.FromHtml("#F0B429");
        public static readonly Color Red     = ColorTranslator.FromHtml("#FF4060");
        public static readonly Color White   = Color.White;

        // Status badge colors
        public static readonly Color ActiveBg   = ColorTranslator.FromHtml("#0A2520");
        public static readonly Color PendingBg  = ColorTranslator.FromHtml("#2A2008");
        public static readonly Color ClosedBg   = ColorTranslator.FromHtml("#141A28");
    }
}
