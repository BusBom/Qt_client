#include "mainwindow.h"
#include "appmanager.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    qDebug() << "🟢 main() 진입";
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Busbomproject_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    AppManager manager;  // ✅ LoginPage → MainWindow 전환을 관리
    manager.show();

    int result = a.exec();  //이벤트 루프 진입 : 마우스클릭, 키보드 입력 등 Qt가 내부적으로 감시
    qDebug() << "🔴 main() 종료, 코드:" << result;
    return result;
}
