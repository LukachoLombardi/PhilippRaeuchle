#include "Shared.h"

namespace Shared {
Logger logger = Logger();

void printPhilipp() {
  Serial.println("\r\n\r\n (                                  (                                         \r\n )\\ )    )     (                    )\\ )                          )  (        \r\n(()/( ( /( (   )\\ (                (()/(    )    (    (        ( /(  )\\   (   \r\n /(_)))\\()))\\ ((_))\\  `  )   `  )   /(_))( /(   ))\\  ))\\   (   )\\())((_) ))\\  \r\n(_)) ((_)\\((_) _ ((_) /(/(   /(/(  (_))  )(_)) /((_)/((_)  )\\ ((_)\\  _  /((_) \r\n| _ \\| |(_)(_)| | (_)((_)_\\ ((_)_\\ | _ \\((_)_ (_)) (_))(  ((_)| |(_)| |(_))   \r\n|  _/| \' \\ | || | | || \'_ \\)| \'_ \\)|   // _` |/ -_)| || |/ _| | \' \\ | |/ -_)  \r\n|_|  |_||_||_||_| |_|| .__/ | .__/ |_|_\\\\__,_|\\___| \\_,_|\\__| |_||_||_|\\___|  \r\n                     |_|    |_|                                             ");
}
void printDiag() {
  char buffer[BUFFER_SIZE] = "";
  //snprintf(buffer, BUFFER_SIZE, "motorStateLeft is %d /n motorStateRight is %d", int(Navigation::driver.isLeftMotorActive()), int(Navigation::driver.isRightMotorActive()));
  logger.printline(buffer, "debug");
}
}