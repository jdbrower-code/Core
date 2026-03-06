using System;
using People;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HelloWorld
{

    internal class Program
    {
        static void Main(string[] args)
        {
            people person = new people();
            person.setName();
            person.setAge();
            person.setHeight();
            person.setWeight();
            Console.WriteLine("Name: " + person.getName());
            Console.WriteLine("Age: " + person.getAge());
            Console.WriteLine("Height: " + person.getHeight());
            Console.WriteLine("Weight: " + person.getWeight());
        }
    }
}
