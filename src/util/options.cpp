// #include <iostream>
// #include <mutex>
// #include <options.hpp>
//
// Options::Options(int argc, char* argv[]) {
//     std::call_once(init_flag, [&]() { parseOptions(argc, argv); });
// }
//
// void Options::parseOptions(int argc, char* argv[]) {
//     reportIncorret(argc, argv);
//     parseBufferCount(argc, argv);
//     parseRecordsPerPage(argc, argv);
//     parseHelp(argc, argv);
// }
//
// void Options::parseHelp(int argc, char* argv[]) {}
//
// void Options::printHelp() {
//     std::cout << "Options:\n"
//                  "  -n=[n]  n>2 buffers (required)\n"
//                  "  -b=[b]  b>0 blocking factor (required) \n"
//                  "  -p      disable printing after every phase (optional)\n";
// }
//
// void Options::parseBufferCount(int argc, char* argv[]) {}
// void Options::parseRecordsPerPage(int argc, char* argv[]) {}
// void Options::parsePrinting(int argc, char* argv[]) {}
