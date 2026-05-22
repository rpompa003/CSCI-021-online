/***************************************
Program Name: Pest Control
Author: [Rafael Pompa]
Date Completed: [5/29/2026]
Description: This program will parse an XML file containing a list of bugs based on date and impact level,
using a BST to assign a developer 3 bugs and unassigned bugs to into an XML file
based on the number of developers given from a command line argument.
***************************************/

#include <iostream>
#include <fstream>
#include <string>
#include "pugixml.hpp"
#include "BinarySearchTree.hpp"
#include "LinkedList.hpp"

using namespace std;

// Setting up the variables to parse information from the xml file
struct Bug
{
    string id;
    string description;
    string impact;
    string created; // ISO 8601 format e.g. "20180125T011030Z"

    // Parameterized constructor
    Bug(string bug_id, string information, string level, string date)
        : id(bug_id), description(information), impact(level), created(date) {}

    // Compare bugs by created date -- oldest date comes first in the BST
    bool operator<(const Bug &other) const
    {
        return created < other.created;
    }

    // Returns true if this bug was created after the other bug
    bool operator>(const Bug &other) const
    {
        return created > other.created;
    }

    // Returns false so duplicate dates are still inserted into the BST
    bool operator==(const Bug &other) const
    {
        return false;
    }
};


// Outputting the bug information into the report.xml
void writeBug(ofstream &outFile, const Bug &bug, string indent)
{
    outFile << indent << "<bug id=\"" << bug.id << "\">\n";
    outFile << indent << "    <description>" << bug.description << "</description>\n";
    outFile << indent << "    <impact>" << bug.impact << "</impact>\n";
    outFile << indent << "    <created>" << bug.created << "</created>\n";
    outFile << indent << "</bug>\n";
}


int main(int argc, char* argv[])
{
    // Get input file and number of developers from command line arguments
    if (argc < 3)
    {
        cout << "Please enter the number of developers as a command line argument." << endl;
        return 1;
    }

    string inputFile  = argv[1];
    // stoi converts the command line argument from a string to an integer
    int numDevelopers = stoi(argv[2]);

    // Instantiate an xml document
    pugi::xml_document doc;

    // Checking to see if the program can open the xml file
    if (!doc.load_file(inputFile.c_str()))
    {
        cerr << "Problem opening xml file \"" << inputFile << "\"\n";
        return 1;
    }

    pugi::xml_node root = doc.first_child();

    // Creating a BST for each impact level based on oldest date
    // The leftmost node in each BST will always be the oldest bug
    BinarySearchTree<Bug> highBST;
    BinarySearchTree<Bug> mediumBST;
    BinarySearchTree<Bug> lowBST;

    // Loop through all bugs in the xml file
    for (pugi::xml_node bugNode : root.children("bug"))
    {
        // Get the bug id
        string id = bugNode.attribute("id").value();

        // Get the bug information from the xml file
        string description = bugNode.child_value("description");
        string impact      = bugNode.child_value("impact");
        string created     = bugNode.child_value("created");

        // Create a bug object from the xml data
        Bug bug(id, description, impact, created);

        // Insert the bug into the correct BST based on impact level
        if (bug.impact == "high")
        {
            highBST.insert(bug);
        }
        else if (bug.impact == "medium")
        {
            mediumBST.insert(bug);
        }
        else if (bug.impact == "low")
        {
            lowBST.insert(bug);
        }
    }

    // Store bug assignments for each developer in a linked list
    LinkedList<LinkedList<Bug>> developerAssignments;

    for (int dev = 0; dev < numDevelopers; ++dev)
    {
        LinkedList<Bug> assignments;

        // Get and remove the oldest high impact bug from BST 1
        const Bug* highBug = highBST.findMinimum();
        if (highBug != nullptr)
        {
            assignments.pushBack(*highBug);
            highBST.remove(*highBug);
        }

        // Get and remove the oldest medium impact bug from BST 2
        const Bug* medBug = mediumBST.findMinimum();
        if (medBug != nullptr)
        {
            assignments.pushBack(*medBug);
            mediumBST.remove(*medBug);
        }

        // Get and remove the oldest low impact bug from BST 3
        const Bug* lowBug = lowBST.findMinimum();
        if (lowBug != nullptr)
        {
            assignments.pushBack(*lowBug);
            lowBST.remove(*lowBug);
        }

        developerAssignments.pushBack(assignments);
    }

    // Open output file for writing
    string outputFile = "report.xml";
    ofstream outFile;
    outFile.open(outputFile, ofstream::trunc);

    // Check if the output file opened successfully
    if (!outFile.is_open())
    {
        cerr << "Error: could not open output file \"" << outputFile << "\"" << endl;
        return 1;
    }

    // Write XML declaration and opening report tag
    outFile << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    outFile << "<report>\n";

    // Write each developer and their assigned bugs
    for (int dev = 0; dev < numDevelopers; ++dev)
    {
        outFile << "    <developer id=\"" << dev + 1 << "\">\n";

        LinkedList<Bug> assignments = developerAssignments.at(dev);
        for (size_t i = 0; i < assignments.size(); ++i)
        {
            writeBug(outFile, assignments.at(i), "        ");
        }

        outFile << "    </developer>\n";
    }

    // Write remaining unassigned bugs to <remaining> section
    outFile << "    <remaining>\n";

    LinkedList<Bug> remainingHigh   = highBST.getSortedList();
    LinkedList<Bug> remainingMedium = mediumBST.getSortedList();
    LinkedList<Bug> remainingLow    = lowBST.getSortedList();

    for (size_t i = 0; i < remainingHigh.size(); ++i)
        writeBug(outFile, remainingHigh.at(i), "        ");

    for (size_t i = 0; i < remainingMedium.size(); ++i)
        writeBug(outFile, remainingMedium.at(i), "        ");

    for (size_t i = 0; i < remainingLow.size(); ++i)
        writeBug(outFile, remainingLow.at(i), "        ");

    outFile << "    </remaining>\n";
    outFile << "</report>";
    outFile.close();

    cout << "The Output was written to " << outputFile << endl;

    return 0;
}