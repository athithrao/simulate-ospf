//
// Created by Athith Amarnath on 12/3/16.
//

#include "logger.h"

// Use the namespace you want

        FileLogger::FileLogger (const int log_type, const char *engine_version, const char *fname = "ige_log.txt")
                :   numWarnings (0U),
                    numErrors (0U)
        {

            myFile.open (fname);

            // Write the first lines
            if(log_type == 0)
            {
                if (myFile.is_open()) {
                    myFile << "Router Log file " << engine_version << std::endl;
                    myFile << "Log file created" << std::endl << std::endl;
                }
            }
            else
            {
                if (myFile.is_open()) {
                    myFile << "Manager Log File" << engine_version << std::endl;
                    myFile << "Log file created" << std::endl << std::endl;
                }
            }
            // if

        }

        FileLogger::~FileLogger ()
        {   

            if (myFile.is_open()) {
                myFile << std::endl << std::endl;

                // Report number of errors and warnings
                myFile << numWarnings << " warnings" << std::endl;
                myFile << numErrors << " errors" << std::endl;

                myFile.close();
            } // if

        }

        // Overl    oad << operator using log type