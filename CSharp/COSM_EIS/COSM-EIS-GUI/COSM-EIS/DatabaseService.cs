using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace COSM_EIS
{
    public enum SearchOperator { And, Or }

    public class SearchCriterion
    {
        public string Field    { get; set; } = "Name";
        public string Operator { get; set; } = "contains";
        public string Value    { get; set; } = "";
    }

    /// <summary>
    /// In-memory database of 180 sample records with multi-criteria search support.
    /// </summary>
    public static class DatabaseService
    {
        private static readonly List<DataRecord> _records;

        private static readonly string[] Categories = { "Analytics", "Infrastructure", "Security", "DevOps", "Finance", "Marketing", "Operations" };
        private static readonly string[] Regions    = { "North", "South", "East", "West", "Central" };
        private static readonly string[] Statuses   = { "active", "pending", "closed" };
        private static readonly string[] Names =
        {
            "Apex Solutions","Bright Data Co","Cobalt Systems","Delta Force","Echo Networks",
            "Falcon Tech","Gamma Labs","Horizon Corp","Iris Digital","Jade Software",
            "Karma Cloud","Luna Services","Matrix Inc","Nova Platform","Omega Analytics",
            "Pulse AI","Quantum Edge","Radix Group","Solar Dynamics","Titan Works",
            "Unified Systems","Vector Space","Wavefront","Xenon Labs","Yellow Brick","Zeta Prime"
        };

        static DatabaseService()
        {
            var rng   = new Random(42);
            var start = new DateTime(2023, 1, 1);
            _records  = new List<DataRecord>(180);

            for (int i = 0; i < 180; i++)
            {
                string baseName = Names[i % Names.Length];
                string name     = i >= Names.Length ? $"{baseName} {i / Names.Length + 1}" : baseName;

                _records.Add(new DataRecord
                {
                    Id       = $"REC-{i + 1:D4}",
                    Name     = name,
                    Category = Categories[rng.Next(Categories.Length)],
                    Region   = Regions[rng.Next(Regions.Length)],
                    Status   = Statuses[rng.Next(Statuses.Length)],
                    Value    = rng.Next(2000, 100000),
                    Score    = rng.Next(0, 101),
                    Date     = start.AddDays(rng.Next(730))
                });
            }
        }

        public static IReadOnlyList<DataRecord> AllRecords => _records;

        /// <summary>
        /// Filters records using up to 10 criteria combined with AND or OR.
        /// </summary>
        public static List<DataRecord> Search(IEnumerable<SearchCriterion> criteria, SearchOperator op)
        {
            var activeCriteria = criteria
                .Where(c => !string.IsNullOrWhiteSpace(c.Value))
                .ToList();

            if (activeCriteria.Count == 0)
                return _records.ToList();

            return _records.Where(r =>
            {
                var matches = activeCriteria.Select(c => Matches(r, c));
                return op == SearchOperator.And ? matches.All(m => m) : matches.Any(m => m);
            }).ToList();
        }

        private static bool WildcardMatch(string input, string pattern)
        {
            string regexPattern = "^" + Regex.Escape(pattern)
                .Replace(@"\*", ".*")
                .Replace(@"\?", ".") + "$";
            return Regex.IsMatch(input, regexPattern, RegexOptions.IgnoreCase);
        }

        private static bool Matches(DataRecord r, SearchCriterion c)
        {
            string fieldValue = GetFieldValue(r, c.Field);
            string query      = c.Value.Trim().ToLowerInvariant();
            string fvLower    = fieldValue.ToLowerInvariant();

            // Numeric comparison
            bool isNumericField = c.Field is "Value" or "Score";
            if (isNumericField && decimal.TryParse(query, out decimal qNum))
            {
                decimal fNum = decimal.Parse(fieldValue);
                return c.Operator switch
                {
                    "="  or "equals"     => fNum == qNum,
                    "≠"  or "not equals" => fNum != qNum,
                    ">"                  => fNum > qNum,
                    "<"                  => fNum < qNum,
                    "≥"                  => fNum >= qNum,
                    "≤"                  => fNum <= qNum,
                    "between"            => ParseBetween(query, out decimal lo, out decimal hi) && fNum >= lo && fNum <= hi,
                    _                    => fvLower.Contains(query)
                };
            }

            // Date comparison
            if (c.Field == "Date" && DateTime.TryParse(query, out DateTime qDate))
            {
                return c.Operator switch
                {
                    "equals" or "="  => r.Date.Date == qDate.Date,
                    "before"         => r.Date.Date < qDate.Date,
                    "after"          => r.Date.Date > qDate.Date,
                    "between"        => ParseBetweenDate(query, out DateTime d1, out DateTime d2) && r.Date.Date >= d1 && r.Date.Date <= d2,
                    _                => fvLower.Contains(query)
                };
            }

            // String comparison
            return c.Operator switch
            {
                "contains" => fvLower.Contains(query) || WildcardMatch(fvLower, query),
                "not contains" => !fvLower.Contains(query) && !WildcardMatch(fvLower, query),
                "equals" => fvLower == query || WildcardMatch(fvLower, query),
                "not equals" => fvLower != query && !WildcardMatch(fvLower, query),
                "starts with" => fvLower.StartsWith(query) || WildcardMatch(fvLower, query),
                "ends with" => fvLower.EndsWith(query) || WildcardMatch(fvLower, query),
                _ => fvLower.Contains(query) || WildcardMatch(fvLower, query)
            };
        }

        private static string GetFieldValue(DataRecord r, string field) => field switch
        {
            "Name"     => r.Name,
            "Category" => r.Category,
            "Region"   => r.Region,
            "Status"   => r.Status,
            "Value"    => r.Value.ToString("F0"),
            "Score"    => r.Score.ToString(),
            "Date"     => r.DateDisplay,
            _          => r.Name
        };

        private static bool ParseBetween(string s, out decimal lo, out decimal hi)
        {
            lo = hi = 0;
            var parts = s.Split(',');
            if (parts.Length < 2) return false;
            return decimal.TryParse(parts[0].Trim(), out lo) && decimal.TryParse(parts[1].Trim(), out hi);
        }

        private static bool ParseBetweenDate(string s, out DateTime d1, out DateTime d2)
        {
            d1 = d2 = DateTime.MinValue;
            var parts = s.Split(',');
            if (parts.Length < 2) return false;
            return DateTime.TryParse(parts[0].Trim(), out d1) && DateTime.TryParse(parts[1].Trim(), out d2);
        }
    }
}
