using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;

namespace COSM_EIS
{
    /// <summary>
    /// A single search criterion row: [#N] [Field ▾] [Operator ▾] [Value____] [✕]
    /// </summary>
    public class CriterionRow : UserControl
    {
        public event EventHandler? RemoveRequested;

        private static int _counter = 0;
        private readonly int _rowNumber;

        private readonly Label    _numLabel;
        private readonly ComboBox _fieldBox;
        private readonly ComboBox _opBox;
        private readonly TextBox  _valueBox;
        private readonly Button   _removeBtn;

        //TO DO
        //UPDATE THE FIELDS BELOW TO MATCH ACTUAL EIS SEARCH ITEMS
        private static readonly Dictionary<string, string[]> OperatorsByField = new()
        {
            ["Name"]     = new[] { "contains", "not contains", "equals", "not equals", "starts with", "ends with" },
            ["Category"] = new[] { "equals", "not equals" },
            ["Region"]   = new[] { "equals", "not equals" },
            ["Status"]   = new[] { "equals", "not equals" },
            ["Value"]    = new[] { "=", "≠", ">", "<", "≥", "≤", "between" },
            ["Score"]    = new[] { "=", "≠", ">", "<", "≥", "≤", "between" },
            ["Date"]     = new[] { "equals", "before", "after", "between" },
        };

        public CriterionRow()
        {
            _rowNumber = ++_counter;

            Height      = 72;
            BackColor   = AppColors.Surf3;
            Padding     = new Padding(10, 6, 10, 6);
            Cursor      = Cursors.Default;

            // Row number label
            _numLabel = new Label
            {
                Text      = $"#{_rowNumber}",
                ForeColor = AppColors.Muted,
                Font      = new Font("Consolas", 8f),
                AutoSize  = true,
                Location  = new Point(10, 6)
            };

            // Field combo
            //TO DO
            //UPDATE THIS FIELD BOX TO MATCH COSM EIS SEARCH OPTIONS
            _fieldBox = CreateCombo(new[] { "Name", "Category", "Region", "Status", "Value", "Score", "Date" });
            _fieldBox.SelectedIndex = 0;
            _fieldBox.SelectedIndexChanged += (s, e) => RefreshOperators();

            // Operator combo
            _opBox = CreateCombo(OperatorsByField["Name"]);
            _opBox.SelectedIndex = 0;

            // Value textbox
            _valueBox = new TextBox
            {
                BackColor   = AppColors.Surf2,
                ForeColor   = AppColors.Text,
                BorderStyle = BorderStyle.None,
                Font        = new Font("Consolas", 10f),
                Cursor      = Cursors.IBeam,
            };
            _valueBox.Enter += (s, e) => Invalidate();
            _valueBox.Leave += (s, e) => Invalidate();

            // Remove button
            _removeBtn = new Button
            {
                Text      = "✕",
                FlatStyle = FlatStyle.Flat,
                BackColor = AppColors.Surf2,
                ForeColor = AppColors.Muted,
                Font      = new Font("Segoe UI", 9f),
                Cursor    = Cursors.Hand,
                Size      = new Size(28, 28),
            };
            _removeBtn.FlatAppearance.BorderColor = AppColors.Border2;
            _removeBtn.FlatAppearance.BorderSize  = 1;
            _removeBtn.Click += (s, e) => RemoveRequested?.Invoke(this, EventArgs.Empty);
            _removeBtn.MouseEnter += (s, e) => { _removeBtn.ForeColor = AppColors.Red; _removeBtn.FlatAppearance.BorderColor = AppColors.Red; };
            _removeBtn.MouseLeave += (s, e) => { _removeBtn.ForeColor = AppColors.Muted; _removeBtn.FlatAppearance.BorderColor = AppColors.Border2; };

            Controls.AddRange(new Control[] { _numLabel, _fieldBox, _opBox, _valueBox, _removeBtn });

            SizeChanged += (s, e) => LayoutControls();
            LayoutControls();
        }

        private static ComboBox CreateCombo(string[] items)
        {
            var cb = new ComboBox
            {
                DropDownStyle = ComboBoxStyle.DropDownList,
                BackColor     = AppColors.Surf2,
                ForeColor     = AppColors.Text,
                FlatStyle     = FlatStyle.Flat,
                Font          = new Font("Consolas", 10f),
            };
            cb.Items.AddRange(items);
            return cb;
        }

        private void LayoutControls()
        {
            int w   = Width;
            int y0  = 22;
            int h   = 28;
            int rem = 32;

            int available = w - 20 - rem - 12;   // total width minus padding and remove btn
            int fw  = (int)(available * 0.28);
            int ow  = (int)(available * 0.22);
            int vw  = available - fw - ow - 16;

            _fieldBox.SetBounds(10,        y0, fw,  h);
            _opBox   .SetBounds(10+fw+8,   y0, ow,  h);
            _valueBox.SetBounds(10+fw+ow+16, y0+4, vw, h-8);
            _removeBtn.SetBounds(w-rem-6,  y0, rem, h);
        }

        private void RefreshOperators()
        {
            string field = _fieldBox.SelectedItem?.ToString() ?? "Name";
            string[] ops = OperatorsByField.TryGetValue(field, out var o) ? o : new[] { "contains" };
            _opBox.Items.Clear();
            _opBox.Items.AddRange(ops);
            _opBox.SelectedIndex = 0;
        }

        public SearchCriterion GetCriterion() => new()
        {
            Field    = _fieldBox.SelectedItem?.ToString() ?? "Name",
            Operator = _opBox.SelectedItem?.ToString()    ?? "contains",
            Value    = _valueBox.Text
        };

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            var g   = e.Graphics;
            g.SmoothingMode = SmoothingMode.AntiAlias;

            bool focused = _valueBox.Focused;
            var borderCol = focused ? AppColors.Accent : AppColors.Border2;

            using var pen = new Pen(borderCol, 1);
            DrawRoundRect(g, pen, 0, 0, Width - 1, Height - 1, 8);

            // Value box border
            int vx = _valueBox.Left - 6;
            int vy = _valueBox.Top  - 5;
            int vw = _valueBox.Width + 12;
            int vh = _valueBox.Height + 10;
            using var vPen = new Pen(focused ? AppColors.Accent : AppColors.Border2, 1);
            DrawRoundRect(g, vPen, vx, vy, vw, vh, 5);

            if (focused)
            {
                using var glowPen = new Pen(Color.FromArgb(40, AppColors.Accent), 3);
                DrawRoundRect(g, glowPen, vx - 1, vy - 1, vw + 2, vh + 2, 6);
            }
        }

        private static void DrawRoundRect(Graphics g, Pen pen, int x, int y, int w, int h, int r)
        {
            using var path = new GraphicsPath();
            path.AddArc(x, y, r * 2, r * 2, 180, 90);
            path.AddArc(x + w - r * 2, y, r * 2, r * 2, 270, 90);
            path.AddArc(x + w - r * 2, y + h - r * 2, r * 2, r * 2, 0, 90);
            path.AddArc(x, y + h - r * 2, r * 2, r * 2, 90, 90);
            path.CloseFigure();
            g.DrawPath(pen, path);
        }

        public static void ResetCounter() => _counter = 0;
    }
}
