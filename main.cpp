/* KRPINTER4 - Simple PostScript document printer
 * Copyright (C) 2014 Marco Nelles, credativ GmbH (marco.nelles@credativ.de)
 * <http://www.credativ.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QtGui/QPainter>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <kdeprintdialog.h>
#include <kprintpreview.h>
#include <kdebug.h>
#include <KLocale>

#include "config.h"
#include "psdocument.h"
#include "fileprinter.h"

#include "widgets/printscalingoptionswidget.h"

/* Return codes:
 * 1: No parameters given. Exit.
 * 2: No PostScript file(s) given. Exit.
 * 3: All PostScript file(s) invalid. Exit.
 */

int showPrintDialogAndPrint(const QString &filename,
                            const QString& printername,
                            const QString& printtitle,
                            const int numCopies,
                            const QStringList& printerOptions,
                            bool nodialog,
                            const QString& system) {

  PSDocument doc;
  if (!doc.load(filename)) {
    kDebug() << "Loading of document " << filename << " failed.";
    return -1;
  }
  int numPages = doc.numPages();
  QPrinter::PaperSize paperSize = doc.paperSize();

  QPrinter printer;
  printer.setPaperSize(paperSize);
  if (!printername.isEmpty()) printer.setPrinterName(printername);
  if (!printtitle.isEmpty()) printer.setDocName(printtitle);
  printer.setCopyCount(numCopies);


  QString pageRange;
  if ((printer.fromPage() > 0) && (printer.toPage() > 0))
    pageRange = QString("%1-%2").arg(printer.fromPage()).arg(printer.toPage());

  printScalingOptionsWidget scaleWidget;

  QPrintDialog *printDialog = KdePrint::createPrintDialog(&printer, QList<QWidget*>() << &scaleWidget);
  printDialog->setWindowTitle(i18n("KPrinter4"));
  if (numPages > 0) {
    printDialog->setMinMax(1, numPages);
    printDialog->setFromTo(1, numPages);
  }

  int ret = 0;
  if ((nodialog) || printDialog->exec()) {

    ret = FilePrinter::printFiles(printer, QStringList(filename),
      QPrinter::Portrait,
      FilePrinter::ApplicationDeletesFiles,
      FilePrinter::SystemSelectsPages,
      pageRange,
      printerOptions,
      system);

  }

  return ret;

}

int main(int argc, char *argv[]) {

  KAboutData aboutData("kprinter4", 0, ki18n("KPrinter4"), KPRINTER4_VERSION,
                       ki18n("Simple PostScript document printer"),
                       KAboutData::License_GPL,
                       ki18n("Copyright © 2014 by Marco Nelles (credativ GmbH)"),
                       KLocalizedString(),
                       "http://www.credativ.com/",
                       "marco.nelles@credativ.de");

  KCmdLineArgs::init(argc, argv, &aboutData);

  KCmdLineOptions options;
  options.add("c", ki18n("Make an internal copy of the files to print"));
  options.add("p").add("D <printer>", ki18n("Printer/destination to print on"));
  options.add("J").add("t <title>", ki18n("Title/Name for the print job"));
  options.add("#").add("n <number>", ki18n("Number of copies"), "1");
  options.add("o <argument> <option=value>", ki18n("Printer/Job option(s)"));
  options.add("j <mode>", ki18n("Job output mode (gui, console, none)"), "gui");
  options.add("system <printsys>", ki18n("Print system to use (autodetect, lpd, cups)"), "autodetect");
  options.add("stdin", ki18n("Allow printing from STDIN"));
  options.add("nd").add("nodialog", ki18n("Do not show the print dialog (print directly)"));
  options.add("+[file(s)]", ki18n("PostScript document(s) to print"));
  KCmdLineArgs::addCmdLineOptions(options);

  KApplication app;

  /* Parsing command line arguments */

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if (args->isSet("c")) kWarning() << i18n("Option -c not implemented yet");
  if (args->isSet("stdin")) kWarning() << i18n("Option --stdin not implemented yet");

  QString printer = args->getOption("D");

  QString title = args->getOption("t");

  int numCopies = 1;
  if (!args->getOption("n").isEmpty()) {
    bool ok;
    int i = args->getOption("n").toInt(&ok);
    if (ok) numCopies = i;
  }

  QStringList printerOptions = args->getOptionList("o");

  QString output_mode = args->getOption("j");
  if ((output_mode != "gui") && (output_mode != "console") && (output_mode != "none")) {
    kWarning() << i18n("Unknown value \"%1\" for option -j. Using default value \"gui\".", output_mode);
    output_mode = "gui";
  }

  QString system = args->getOption("system");
  if ((system != "autodetect") && (system != "cups") && (system != "lpd")) {
    kWarning() << i18n("Unknown value \"%1\" for option --system. Using default value \"autodetect\".", system);
    system = "autodetect";
  }

  bool nodialog = ((args->getOption("j") != "gui") || !args->isSet("nd"));

  int ret = 1;
  if (args->count()) {

    QStringList psfilenames;
    int ok = 0;
    for (int i = 0; i < args->count(); ++i)
      if (showPrintDialogAndPrint(args->url(i).path(),
        args->getOption("D"),
        args->getOption("t"),
        numCopies,
        printerOptions,
        nodialog,
        system) == 0) {
          ++ok;
      }

    if (ok == 0) ret = 3; else ret = 0;

  }

  return ret;

}
