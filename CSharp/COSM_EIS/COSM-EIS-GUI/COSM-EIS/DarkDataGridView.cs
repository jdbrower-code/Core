using System.Drawing;
using System.Windows.Forms;

namespace COSM_EIS
{
    /// <summary>
    /// Overrides default DataGridView rendering to match the dark professional theme.
    /// </summary>
    internal class DarkDataGridView : DataGridView
    {
        public DarkDataGridView()
        {
            DoubleBuffered          = true;
            BorderStyle             = BorderStyle.None;
            BackgroundColor         = AppColors.Surf;
            GridColor               = AppColors.Border;
            RowHeadersVisible       = false;
            AllowUserToAddRows      = false;
            AllowUserToDeleteRows   = false;
            AllowUserToResizeRows   = false;
            SelectionMode           = DataGridViewSelectionMode.FullRowSelect;
            MultiSelect             = true;
            ReadOnly                = true;
            AutoSizeRowsMode        = DataGridViewAutoSizeRowsMode.None;
            RowTemplate.Height      = 40;
            ColumnHeadersHeight     = 36;
            ColumnHeadersHeightSizeMode = DataGridViewColumnHeadersHeightSizeMode.DisableResizing;
            EnableHeadersVisualStyles = false;
            CellBorderStyle         = DataGridViewCellBorderStyle.SingleHorizontal;

            // Default cell style
            DefaultCellStyle.BackColor         = AppColors.Surf;
            DefaultCellStyle.ForeColor         = AppColors.Text;
            DefaultCellStyle.SelectionBackColor = Color.FromArgb(30, AppColors.Accent);
            DefaultCellStyle.SelectionForeColor = AppColors.Text;
            DefaultCellStyle.Font              = new Font("Consolas", 10f);
            DefaultCellStyle.Padding           = new Padding(6, 0, 6, 0);

            // Alternating rows
            AlternatingRowsDefaultCellStyle.BackColor          = Color.FromArgb(15, 21, 32);
            AlternatingRowsDefaultCellStyle.SelectionBackColor = Color.FromArgb(30, AppColors.Accent);

            // Column header style
            ColumnHeadersDefaultCellStyle.BackColor  = AppColors.Surf2;
            ColumnHeadersDefaultCellStyle.ForeColor  = AppColors.Muted2;
            ColumnHeadersDefaultCellStyle.Font       = new Font("Consolas", 9f, FontStyle.Bold);
            ColumnHeadersDefaultCellStyle.SelectionBackColor = AppColors.Surf2;
            ColumnHeadersDefaultCellStyle.Padding    = new Padding(6, 0, 6, 0);
        }

        protected override void OnCellPainting(DataGridViewCellPaintingEventArgs e)
        {
            if (e.RowIndex == -1)
            {
                // Header row
                e.Graphics.FillRectangle(new SolidBrush(AppColors.Surf2), e.CellBounds);
                if (e.ColumnIndex > 0)
                    e.Graphics.DrawLine(new Pen(AppColors.Border, 1),
                        e.CellBounds.Left, e.CellBounds.Top + 6,
                        e.CellBounds.Left, e.CellBounds.Bottom - 6);

                // Header text
                string txt = e.Value?.ToString() ?? "";
                using var sf = new StringFormat { Alignment = StringAlignment.Near, LineAlignment = StringAlignment.Center };
                e.Graphics.DrawString(txt, ColumnHeadersDefaultCellStyle.Font!, new SolidBrush(AppColors.Muted2), e.CellBounds, sf);

                // Sort glyph
                if (e.ColumnIndex >= 0 && SortedColumn != null && SortedColumn.Index == e.ColumnIndex)
                {
                    string arrow = SortOrder == SortOrder.Ascending ? " ↑" : " ↓";
                    var sz  = e.Graphics.MeasureString(txt, ColumnHeadersDefaultCellStyle.Font!);
                    var arrowRect = new RectangleF(e.CellBounds.Left + sz.Width + 4, e.CellBounds.Top,
                                                   20, e.CellBounds.Height);
                    e.Graphics.DrawString(arrow, ColumnHeadersDefaultCellStyle.Font!,
                        new SolidBrush(AppColors.Accent), arrowRect, sf);
                }

                e.Graphics.DrawLine(new Pen(AppColors.Border, 1),
                    e.CellBounds.Left, e.CellBounds.Bottom - 1,
                    e.CellBounds.Right, e.CellBounds.Bottom - 1);

                e.Handled = true;
                return;
            }

            // Status column — colored pill badge
            if (Columns[e.ColumnIndex].Name == "Status" && e.Value != null)
            {
                var bg = e.RowIndex % 2 == 0 ? AppColors.Surf : Color.FromArgb(15, 21, 32);
                if (SelectedRows.Contains(Rows[e.RowIndex]))
                    bg = Color.FromArgb(30, AppColors.Accent.R, AppColors.Accent.G, AppColors.Accent.B);
                e.Graphics.FillRectangle(new SolidBrush(bg), e.CellBounds);

                string status = e.Value.ToString()!.ToLower();
                (Color pillBg, Color pillFg) = status switch
                {
                    "active"  => (AppColors.ActiveBg,  AppColors.Accent2),
                    "pending" => (AppColors.PendingBg, AppColors.Gold),
                    _         => (AppColors.ClosedBg,  AppColors.Muted2)
                };

                string label = status.ToUpper();
                var   fnt    = new Font("Consolas", 8f, FontStyle.Bold);
                var   sz     = e.Graphics.MeasureString(label, fnt);
                float px     = 8f, py = 4f;
                float bx = e.CellBounds.Left + 6;
                float by = e.CellBounds.Top  + (e.CellBounds.Height - sz.Height - py * 2) / 2f;
                float bw = sz.Width + px * 2;
                float bh = sz.Height + py * 2;

                using var path  = RoundedRect(bx, by, bw, bh, bh / 2);
                e.Graphics.FillPath(new SolidBrush(pillBg), path);
                e.Graphics.DrawString(label, fnt, new SolidBrush(pillFg), bx + px, by + py);

                e.Graphics.DrawLine(new Pen(AppColors.Border, 1),
                    e.CellBounds.Left, e.CellBounds.Bottom - 1,
                    e.CellBounds.Right, e.CellBounds.Bottom - 1);

                e.Handled = true;
                return;
            }

            // Score column — progress bar
            if (Columns[e.ColumnIndex].Name == "Score" && e.Value != null && int.TryParse(e.Value.ToString(), out int score))
            {
                var bg = e.RowIndex % 2 == 0 ? AppColors.Surf : Color.FromArgb(15, 21, 32);
                if (SelectedRows.Contains(Rows[e.RowIndex]))
                    bg = Color.FromArgb(30, AppColors.Accent.R, AppColors.Accent.G, AppColors.Accent.B);
                e.Graphics.FillRectangle(new SolidBrush(bg), e.CellBounds);

                int bx  = e.CellBounds.Left + 6;
                int by  = e.CellBounds.Top  + e.CellBounds.Height / 2 - 3;
                int bw  = e.CellBounds.Width - 44;
                int bh  = 6;
                int fill = (int)(bw * score / 100.0);

                Color barCol = score >= 70 ? AppColors.Accent2 : score >= 40 ? AppColors.Gold : AppColors.Red;

                e.Graphics.FillRectangle(new SolidBrush(AppColors.Surf3), bx, by, bw, bh);
                if (fill > 0) e.Graphics.FillRectangle(new SolidBrush(barCol), bx, by, fill, bh);

                using var sf = new StringFormat { Alignment = StringAlignment.Far, LineAlignment = StringAlignment.Center };
                e.Graphics.DrawString(score.ToString(), new Font("Consolas", 9f),
                    new SolidBrush(AppColors.Muted2),
                    new Rectangle(e.CellBounds.Left, e.CellBounds.Top, e.CellBounds.Width - 4, e.CellBounds.Height), sf);

                e.Graphics.DrawLine(new Pen(AppColors.Border, 1),
                    e.CellBounds.Left, e.CellBounds.Bottom - 1,
                    e.CellBounds.Right, e.CellBounds.Bottom - 1);

                e.Handled = true;
                return;
            }

            base.OnCellPainting(e);
        }

        private static System.Drawing.Drawing2D.GraphicsPath RoundedRect(float x, float y, float w, float h, float r)
        {
            var path = new System.Drawing.Drawing2D.GraphicsPath();
            path.AddArc(x, y, r * 2, r * 2, 180, 90);
            path.AddArc(x + w - r * 2, y, r * 2, r * 2, 270, 90);
            path.AddArc(x + w - r * 2, y + h - r * 2, r * 2, r * 2, 0, 90);
            path.AddArc(x, y + h - r * 2, r * 2, r * 2, 90, 90);
            path.CloseFigure();
            return path;
        }
    }
}
