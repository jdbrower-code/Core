using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace People
{
    internal class people
    {
        private string Name;
        private string Age;
        private string Height;
        private string Weight;

        public void setName()
        {
            Console.WriteLine("Please Type your Name then hit the Enter Key");
            Name = Console.ReadLine();


        }

        public void setAge()
        {
            Console.WriteLine("Please Type your Age then hit the Enter Key");
            Age = Console.ReadLine();
        }

        public void setHeight()
        {
            Console.WriteLine("Please Type your Height then hit the Enter Key");
            Height = Console.ReadLine();
        }

        public void setWeight()
        {
            Console.WriteLine("Please Type your Weight then hit the Enter Key");
            Weight = Console.ReadLine();
        }

        public string getName()
        {
            return Name;
        }

        public string getAge()
        {
            return Age;
        }

        public string getHeight()
        {
            return Height;
        }

        public string getWeight()
        {
            return Weight;
        }
    }
}


