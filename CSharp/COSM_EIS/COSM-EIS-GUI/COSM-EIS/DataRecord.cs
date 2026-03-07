using System;

namespace COSM_EIS
{
    /// <summary>Represents a single record in the in-memory database.</summary>
    public class DataRecord
    {
        public string Id       { get; set; } = "";
        public string Name     { get; set; } = "";
        public string Category { get; set; } = "";
        public string Region   { get; set; } = "";
        public string Status   { get; set; } = "";
        public decimal Value   { get; set; }
        public int Score       { get; set; }
        public DateTime Date   { get; set; }

        public string DateDisplay => Date.ToString("yyyy-MM-dd");
        public string ValueDisplay => $"${Value:N0}";
    }
}
