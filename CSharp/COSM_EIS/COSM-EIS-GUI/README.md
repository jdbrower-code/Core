# COSM-EIS — Enterprise Information Search

A dark, professional database search GUI built with C# WinForms (.NET 6).

## Requirements

- **Visual Studio 2022** (Community, Professional, or Enterprise)
- **.NET 6 SDK** (included with VS 2022)
- Windows (WinForms is Windows-only)

## Opening in Visual Studio

1. Double-click `COSM-EISApp.sln`  
   — or open Visual Studio → **File → Open → Project/Solution** → select `COSM-EISApp.sln`

2. Press **F5** to build and run, or use **Build → Build Solution** (Ctrl+Shift+B)

## Features

| Feature | Description |
|---|---|
| **Up to 10 search criteria** | Add/remove rows dynamically |
| **Smart operators** | Operators change per field type (text, numeric, date) |
| **AND / OR combine** | Toggle how criteria are combined |
| **Live results grid** | Sortable columns, color-coded status pills, score bars |
| **Export CSV** | Save filtered results to a .csv file |
| **180 sample records** | Auto-generated in-memory database |

## Search Fields & Operators

| Field    | Available Operators |
|----------|---------------------|
| Name     | contains, not contains, equals, not equals, starts with, ends with |
| Category | equals, not equals |
| Region   | equals, not equals |
| Status   | equals, not equals |
| Value    | =, ≠, >, <, ≥, ≤, between (e.g. `5000,20000`) |
| Score    | =, ≠, >, <, ≥, ≤, between |
| Date     | equals, before, after, between (e.g. `2023-01-01,2024-01-01`) |

## Project Structure

```
COSM-EISApp/
├── COSM-EISApp.sln          # Visual Studio solution
└── COSM-EISApp/
    ├── COSM-EISApp.csproj   # Project file (.NET 6 WinForms)
    ├── Program.cs            # Entry point
    ├── MainForm.cs           # Main window & layout
    ├── MainForm.Designer.cs  # WinForms designer stub
    ├── CriterionRow.cs       # Individual search criterion control
    ├── DarkDataGridView.cs   # Custom dark-themed grid
    ├── DatabaseService.cs    # In-memory DB + search logic
    ├── DataRecord.cs         # Record model
    └── AppColors.cs          # Centralized color palette
```
