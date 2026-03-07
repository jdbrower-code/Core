using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COSM_EIS
{
    public partial class MainForm : Form
    {
        // ── Layout constants ──────────────────────────────────────────────
        private const int TopBarH    = 56;
        private const int StatusBarH = 32;
        private const int Pad        = 16;
        private const int LeftW      = 420;
        private const int MaxCriteria = 10;

        // ── Search criteria state ─────────────────────────────────────────
        private readonly List<CriterionRow> _criteria = new();
        private SearchOperator _operator = SearchOperator.And;

        // ── Current results ───────────────────────────────────────────────
        private List<DataRecord> _results = new();

        // ── Controls ──────────────────────────────────────────────────────
        private Panel          _topBar        = null!;
        private Panel          _statusBar     = null!;
        private Panel          _leftPanel     = null!;
        private Panel          _rightPanel    = null!;
        private Panel          _criteriaScroll = null!;
        private Button         _btnAnd        = null!;
        private Button         _btnOr         = null!;
        private Button         _btnAdd        = null!;
        private Button         _btnSearch     = null!;
        private Button         _btnClear      = null!;
        private Button         _btnExport     = null!;
        private Label          _lblResultCount = null!;
        private Label          _lblStatusMsg  = null!;
        private DarkDataGridView _grid        = null!;

        public MainForm()
        {
            InitializeComponent();
            BuildUI();
            AddCriterion(); // start with one row
        }

        // ── UI Construction ───────────────────────────────────────────────

        private void BuildUI()
        {
            Text            = "COSM-EIS — Enterprise Information Search";
            Size            = new Size(1400, 820);
            MinimumSize     = new Size(1000, 700);
            StartPosition   = FormStartPosition.CenterScreen;
            BackColor       = AppColors.Bg;
            ForeColor       = AppColors.Text;

            BuildTopBar();
            BuildStatusBar();
            BuildLeftPanel();
            BuildRightPanel();

            Resize += (s, e) => ReLayout();
            ReLayout();
        }

        private void BuildTopBar()
        {
            _topBar = new Panel
            {
                Height    = TopBarH,
                Dock      = DockStyle.Top,
                BackColor = AppColors.Surf2,
            };

            // Logo
            var logo = new Panel { Size = new Size(36, 36), Location = new Point(16, 10), BackColor = AppColors.Accent, Cursor = Cursors.Default };
            logo.Paint += (s, e) =>
            {
                e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;
                using var f = new Font("Consolas", 16f, FontStyle.Bold);
                e.Graphics.DrawString("⊕", f, Brushes.White, 4, 6);
            };
            DrawRoundedPanel(logo);

            var lblTitle = DarkLabel("COSM-EIS", new Font("Segoe UI", 15f, FontStyle.Bold), AppColors.Text);
            lblTitle.Location = new Point(62, 10);
            lblTitle.AutoSize = true;

            var lblSub = DarkLabel("Enterprise Information Search  v1.0", new Font("Consolas", 8f), AppColors.Muted);
            lblSub.Location = new Point(63, 34);
            lblSub.AutoSize = true;

            var lblDb = DarkLabel("DB: records_main", new Font("Consolas", 8f), AppColors.Muted2);
            lblDb.AutoSize = true;
            lblDb.Anchor   = AnchorStyles.Top | AnchorStyles.Right;

            var pConnected = CreateBadge("● CONNECTED", AppColors.Accent2);
            pConnected.Anchor = AnchorStyles.Top | AnchorStyles.Right;

            var lblTotal = DarkLabel($"180 records", new Font("Consolas", 8f), AppColors.Muted2);
            lblTotal.AutoSize = true;
            lblTotal.Anchor   = AnchorStyles.Top | AnchorStyles.Right;

            _topBar.Controls.AddRange(new Control[] { logo, lblTitle, lblSub, lblDb, pConnected, lblTotal });

            // Position right-side controls after adding to form
            _topBar.Layout += (s, e) =>
            {
                int rx = _topBar.Width - 12;
                lblTotal.Location   = new Point(rx - lblTotal.Width, 20);
                pConnected.Location = new Point(lblTotal.Left - pConnected.Width - 10, 16);
                lblDb.Location      = new Point(pConnected.Left - lblDb.Width - 10, 20);
            };

            Controls.Add(_topBar);
        }

        private void BuildStatusBar()
        {
            _statusBar = new Panel
            {
                Height    = StatusBarH,
                Dock      = DockStyle.Bottom,
                BackColor = AppColors.Surf2,
            };

            _lblStatusMsg = DarkLabel("COSM-EIS  ·  Enterprise Information Search  ·  Ready", new Font("Consolas", 8f), AppColors.Muted);
            _lblStatusMsg.Location = new Point(16, 10);
            _lblStatusMsg.AutoSize = true;

            var pNormal = CreateBadge("● System Normal", AppColors.Accent2);
            pNormal.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            _statusBar.Layout += (s, e) => pNormal.Location = new Point(_statusBar.Width - pNormal.Width - 12, 6);

            _statusBar.Controls.AddRange(new Control[] { _lblStatusMsg, pNormal });
            Controls.Add(_statusBar);
        }

        private void BuildLeftPanel()
        {
            _leftPanel = new Panel
            {
                BackColor = AppColors.Surf,
            };

            // Panel header
            var header = new Panel { Height = 44, Dock = DockStyle.Top, BackColor = AppColors.Surf2 };
            var dot    = new Panel { Size = new Size(8, 8), Location = new Point(16, 18), BackColor = AppColors.Accent };
            DrawRoundedPanel(dot, 4);
            var lblHead = DarkLabel("SEARCH CRITERIA", new Font("Consolas", 8f, FontStyle.Bold), AppColors.Muted2);
            lblHead.Location = new Point(32, 14);
            lblHead.AutoSize = true;
            var lblCount = DarkLabel("0 / 10", new Font("Consolas", 8f), AppColors.Muted);
            lblCount.Name     = "lblCritCount";
            lblCount.AutoSize = true;
            lblCount.Anchor   = AnchorStyles.Top | AnchorStyles.Right;
            header.Layout += (s, e) => lblCount.Location = new Point(header.Width - lblCount.Width - 14, 14);
            header.Controls.AddRange(new Control[] { dot, lblHead, lblCount });

            // AND / OR
            var opRow = new Panel { Height = 36, Dock = DockStyle.Top, BackColor = AppColors.Surf, Padding = new Padding(14, 6, 14, 0) };
            var lblOp = DarkLabel("COMBINE WITH", new Font("Consolas", 8f), AppColors.Muted);
            lblOp.Location = new Point(14, 10);
            lblOp.AutoSize = true;

            _btnAnd = CreateToggleButton("AND", true);
            _btnAnd.Location = new Point(128, 4);
            _btnAnd.Click += (s, e) => SetOperator(SearchOperator.And);

            _btnOr = CreateToggleButton("OR", false);
            _btnOr.Location = new Point(182, 4);
            _btnOr.Click += (s, e) => SetOperator(SearchOperator.Or);

            opRow.Controls.AddRange(new Control[] { lblOp, _btnAnd, _btnOr });

            // Scrollable criteria area
            _criteriaScroll = new Panel
            {
                AutoScroll  = true,
                BackColor   = AppColors.Surf,
                Padding     = new Padding(10, 8, 10, 8),
            };

            // Bottom action buttons
            var actionRow = new Panel { Height = 58, Dock = DockStyle.Bottom, BackColor = AppColors.Surf, Padding = new Padding(12, 10, 12, 10) };

            _btnAdd = CreateActionButton("＋  Add Criterion", AppColors.Surf2, AppColors.Muted2);
            _btnAdd.Click += (s, e) => AddCriterion();
            _btnAdd.Location = new Point(12, 10);

            _btnSearch = CreateActionButton("⌕   Run Search", AppColors.Accent, AppColors.White);
            _btnSearch.Click += (s, e) => RunSearch();

            _btnClear = CreateActionButton("✕  Clear", AppColors.Surf2, AppColors.Muted2);
            _btnClear.Click += (s, e) => ClearAll();

            actionRow.Controls.AddRange(new Control[] { _btnAdd, _btnSearch, _btnClear });
            actionRow.Layout += (s, e) =>
            {
                int aw  = actionRow.Width - 24;
                int addW  = (int)(aw * 0.26);
                int srcW  = (int)(aw * 0.48);
                int clrW  = aw - addW - srcW - 16;
                _btnAdd.Size    = new Size(addW, 36);
                _btnSearch.Size = new Size(srcW, 36);
                _btnClear.Size  = new Size(clrW, 36);
                _btnAdd.Location    = new Point(12, 10);
                _btnSearch.Location = new Point(12 + addW + 8, 10);
                _btnClear.Location  = new Point(12 + addW + srcW + 16, 10);
            };

            _leftPanel.Controls.AddRange(new Control[] { header, opRow, _criteriaScroll, actionRow });
            Controls.Add(_leftPanel);
        }

        private void BuildRightPanel()
        {
            _rightPanel = new Panel { BackColor = AppColors.Surf };

            // Panel header / toolbar
            var toolbar = new Panel { Height = 44, Dock = DockStyle.Top, BackColor = AppColors.Surf2 };
            var dot     = new Panel { Size = new Size(8, 8), Location = new Point(16, 18), BackColor = AppColors.Accent2 };
            DrawRoundedPanel(dot, 4);
            var lblHead = DarkLabel("RESULTS", new Font("Consolas", 8f, FontStyle.Bold), AppColors.Muted2);
            lblHead.Location = new Point(32, 14);
            lblHead.AutoSize = true;

            _lblResultCount = DarkLabel("— records", new Font("Consolas", 8f), AppColors.Muted);
            _lblResultCount.Location = new Point(110, 14);
            _lblResultCount.AutoSize = true;

            _btnExport = CreateActionButton("↓  Export CSV", AppColors.Surf3, AppColors.Muted2);
            _btnExport.Size    = new Size(110, 26);
            _btnExport.Anchor  = AnchorStyles.Top | AnchorStyles.Right;
            _btnExport.Click  += (s, e) => ExportCsv();
            toolbar.Layout    += (s, e) => _btnExport.Location = new Point(toolbar.Width - _btnExport.Width - 14, 8);

            toolbar.Controls.AddRange(new Control[] { dot, lblHead, _lblResultCount, _btnExport });

            // Grid
            _grid = new DarkDataGridView { Dock = DockStyle.Fill };
            BuildGridColumns();

            _rightPanel.Controls.Add(_grid);
            _rightPanel.Controls.Add(toolbar);
            Controls.Add(_rightPanel);
        }

        private void BuildGridColumns()
        {
            _grid.Columns.Clear();
            var cols = new (string name, string header, int width, DataGridViewContentAlignment align)[]
            {
                ("Id",       "ID",         80,  DataGridViewContentAlignment.MiddleLeft),
                ("Name",     "NAME",       175, DataGridViewContentAlignment.MiddleLeft),
                ("Category", "CATEGORY",   115, DataGridViewContentAlignment.MiddleLeft),
                ("Region",   "REGION",     90,  DataGridViewContentAlignment.MiddleLeft),
                ("Status",   "STATUS",     90,  DataGridViewContentAlignment.MiddleLeft),
                ("Value",    "VALUE ($)",  100, DataGridViewContentAlignment.MiddleLeft),
                ("Score",    "SCORE",      130, DataGridViewContentAlignment.MiddleLeft),
                ("Date",     "DATE",       100, DataGridViewContentAlignment.MiddleLeft),
            };

            foreach (var (name, header, width, align) in cols)
            {
                _grid.Columns.Add(new DataGridViewTextBoxColumn
                {
                    Name            = name,
                    HeaderText      = header,
                    Width           = width,
                    MinimumWidth    = 60,
                    DefaultCellStyle = { Alignment = align },
                    SortMode        = DataGridViewColumnSortMode.Automatic,
                });
            }

            // ID and Date — muted color
            _grid.Columns["Id"]!.DefaultCellStyle.ForeColor   = AppColors.Muted;
            _grid.Columns["Date"]!.DefaultCellStyle.ForeColor  = AppColors.Muted2;
            // Value — accent color
            _grid.Columns["Value"]!.DefaultCellStyle.ForeColor = AppColors.Accent2;
        }

        // ── Layout ────────────────────────────────────────────────────────

        private void ReLayout()
        {
            int contentY = TopBarH;
            int contentH = ClientSize.Height - TopBarH - StatusBarH;
            int rightX   = Pad + LeftW + Pad;
            int rightW   = ClientSize.Width - rightX - Pad;

            _leftPanel.SetBounds(Pad, contentY + Pad, LeftW, contentH - Pad * 2);
            _rightPanel.SetBounds(rightX, contentY + Pad, rightW, contentH - Pad * 2);

            // Criteria scroll takes remaining space between op row and action row
            int taken = 44 + 36 + 58; // header + opRow + actionRow
            _criteriaScroll.SetBounds(0, 44 + 36, LeftW, _leftPanel.Height - taken);
        }

        // ── Criteria management ───────────────────────────────────────────

        private void AddCriterion()
        {
            if (_criteria.Count >= MaxCriteria) return;

            var row = new CriterionRow
            {
                Width    = _criteriaScroll.Width - _criteriaScroll.Padding.Horizontal - 4,
                Anchor   = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
            };

            row.RemoveRequested += (s, e) => RemoveCriterion(row);

            int y = _criteria.Count * 80;
            row.Location = new Point(0, y);
            _criteria.Add(row);
            _criteriaScroll.Controls.Add(row);

            UpdateCriteriaState();
        }

        private void RemoveCriterion(CriterionRow row)
        {
            _criteriaScroll.Controls.Remove(row);
            _criteria.Remove(row);
            row.Dispose();
            ReflowCriteria();
            UpdateCriteriaState();
        }

        private void ReflowCriteria()
        {
            for (int i = 0; i < _criteria.Count; i++)
                _criteria[i].Location = new Point(0, i * 80);
        }

        private void UpdateCriteriaState()
        {
            int count = _criteria.Count;
            _btnAdd.Enabled = count < MaxCriteria;
            _btnAdd.ForeColor = count < MaxCriteria ? AppColors.Muted2 : AppColors.Muted;

            var lbl = _leftPanel.Controls.Find("lblCritCount", true).FirstOrDefault() as Label;
            if (lbl != null) lbl.Text = $"{count} / {MaxCriteria}";
        }

        private void SetOperator(SearchOperator op)
        {
            _operator = op;
            _btnAnd.BackColor = op == SearchOperator.And ? AppColors.Accent : AppColors.Surf3;
            _btnAnd.ForeColor = op == SearchOperator.And ? AppColors.White  : AppColors.Muted2;
            _btnOr.BackColor  = op == SearchOperator.Or  ? AppColors.Accent : AppColors.Surf3;
            _btnOr.ForeColor  = op == SearchOperator.Or  ? AppColors.White  : AppColors.Muted2;
        }

        private void ClearAll()
        {
            foreach (var r in _criteria.ToList())
            {
                _criteriaScroll.Controls.Remove(r);
                r.Dispose();
            }
            _criteria.Clear();
            CriterionRow.ResetCounter();
            AddCriterion();
            _grid.Rows.Clear();
            _results.Clear();
            _lblResultCount.Text = "— records";
            _lblStatusMsg.Text   = "COSM-EIS  ·  Ready";
        }

        // ── Search ────────────────────────────────────────────────────────

        private void RunSearch()
        {
            Cursor = Cursors.WaitCursor;
            _btnSearch.Text    = "Searching…";
            _btnSearch.Enabled = false;
            Application.DoEvents();

            var timer = System.Diagnostics.Stopwatch.StartNew();

            try
            {
                var criteria = _criteria.Select(r => r.GetCriterion()).ToList();
                _results = DatabaseService.Search(criteria, _operator);

                PopulateGrid(_results);

                timer.Stop();
                double ms = timer.Elapsed.TotalMilliseconds;
                _lblResultCount.Text = $"{_results.Count} record{(_results.Count != 1 ? "s" : "")} matched";
                _lblStatusMsg.Text   = $"COSM-EIS  ·  Query completed in {ms:F0}ms  ·  {_results.Count} result{(_results.Count != 1 ? "s" : "")} found";
            }
            finally
            {
                _btnSearch.Text    = "⌕   Run Search";
                _btnSearch.Enabled = true;
                Cursor = Cursors.Default;
            }
        }

        private void PopulateGrid(List<DataRecord> records)
        {
            _grid.SuspendLayout();
            _grid.Rows.Clear();

            foreach (var r in records)
            {
                _grid.Rows.Add(
                    r.Id,
                    r.Name,
                    r.Category,
                    r.Region,
                    r.Status,
                    r.ValueDisplay,
                    r.Score,
                    r.DateDisplay
                );
            }

            _grid.ResumeLayout();
        }

        // ── Export ────────────────────────────────────────────────────────

        private void ExportCsv()
        {
            if (_results.Count == 0) { MessageBox.Show("No results to export.", "Export", MessageBoxButtons.OK, MessageBoxIcon.Information); return; }

            using var dlg = new SaveFileDialog
            {
                Title      = "Export Results as CSV",
                Filter     = "CSV Files (*.csv)|*.csv",
                FileName   = $"cosm_eis_export_{DateTime.Now:yyyyMMdd_HHmmss}.csv"
            };

            if (dlg.ShowDialog() != DialogResult.OK) return;

            var sb = new StringBuilder();
            sb.AppendLine("ID,Name,Category,Region,Status,Value,Score,Date");
            foreach (var r in _results)
                sb.AppendLine($"{r.Id},{r.Name},{r.Category},{r.Region},{r.Status},{r.Value},{r.Score},{r.DateDisplay}");

            File.WriteAllText(dlg.FileName, sb.ToString());
            _lblStatusMsg.Text = $"Exported {_results.Count} records to {Path.GetFileName(dlg.FileName)}";
        }

        // ── Helpers ───────────────────────────────────────────────────────

        private static Label DarkLabel(string text, Font font, Color color) =>
            new() { Text = text, Font = font, ForeColor = color, BackColor = Color.Transparent, AutoSize = true };

        private static Panel CreateBadge(string text, Color fg)
        {
            var lbl = new Label { Text = text, Font = new Font("Consolas", 8f), ForeColor = fg, BackColor = Color.Transparent, AutoSize = true, Location = new Point(8, 5) };
            var p   = new Panel { Height = 24, BackColor = Color.FromArgb(10, 40, 32), Padding = new Padding(8, 4, 8, 4) };
            p.Width = lbl.PreferredWidth + 20;
            p.Controls.Add(lbl);
            return p;
        }

        private static Button CreateToggleButton(string text, bool active)
        {
            var b = new Button
            {
                Text      = text,
                Size      = new Size(46, 24),
                FlatStyle = FlatStyle.Flat,
                Font      = new Font("Consolas", 8f, FontStyle.Bold),
                BackColor = active ? AppColors.Accent : AppColors.Surf3,
                ForeColor = active ? AppColors.White  : AppColors.Muted2,
                Cursor    = Cursors.Hand,
            };
            b.FlatAppearance.BorderSize  = 0;
            return b;
        }

        private static Button CreateActionButton(string text, Color bg, Color fg)
        {
            var b = new Button
            {
                Text      = text,
                FlatStyle = FlatStyle.Flat,
                BackColor = bg,
                ForeColor = fg,
                Font      = new Font("Segoe UI", 9f, FontStyle.Bold),
                Cursor    = Cursors.Hand,
                Size      = new Size(120, 36),
            };
            b.FlatAppearance.BorderColor = AppColors.Border2;
            b.FlatAppearance.BorderSize  = 1;
            b.MouseEnter += (s, e) => b.FlatAppearance.BorderColor = AppColors.Accent;
            b.MouseLeave += (s, e) => b.FlatAppearance.BorderColor = AppColors.Border2;
            return b;
        }

        private static void DrawRoundedPanel(Panel p, int radius = 8)
        {
            p.Paint += (s, e) =>
            {
                e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;
                using var path = new GraphicsPath();
                int r = radius * 2;
                path.AddArc(0, 0, r, r, 180, 90);
                path.AddArc(p.Width - r, 0, r, r, 270, 90);
                path.AddArc(p.Width - r, p.Height - r, r, r, 0, 90);
                path.AddArc(0, p.Height - r, r, r, 90, 90);
                path.CloseFigure();
                e.Graphics.SetClip(path);
                e.Graphics.FillPath(new SolidBrush(p.BackColor), path);
            };
        }
    }
}
