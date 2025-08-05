#include "mainwindow.h"
#include "appmanager.h"
#include "configmanager.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFontDatabase>


int main(int argc, char *argv[])
{
    qDebug() << "🟢 main() 진입";

    QApplication app(argc, argv);

    ConfigManager::load();

    QStringList fontPaths = {
        ":/01HanwhaB.ttf",
        ":/03HanwhaL.ttf",
        ":/05HanwhaGothicR.ttf",
        ":/06HanwhaGothicL.ttf",
        ":/07HanwhaGothicEL.ttf",
        ":/Pretendard-Regular.ttf"
    };

    QString defaultFontFamily;

    for (const QString &path : fontPaths) {
        int id = QFontDatabase::addApplicationFont(path);
        if (id == -1) {
            qWarning() << "❌ Failed to load font:" << path;
        } else {
            QStringList families = QFontDatabase::applicationFontFamilies(id);
            qDebug() << "✅ Loaded:" << path << "→" << families;
            if (path.contains("hanwhaGothic L")) {
                defaultFontFamily = families.first();
            }
        }
    }

    // 2. 전역 폰트는 Pretendard로만 설정
    if (!defaultFontFamily.isEmpty()) {
        QFont defaultFont(defaultFontFamily, 11);
        QApplication::setFont(defaultFont);
    }


    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Busbomproject_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }
    AppManager manager;  // LoginPage → MainWindow 전환
    manager.show();

    int result = app.exec();  //이벤트 루프 진입 : 마우스클릭, 키보드 입력 등 Qt가 내부적으로 감시
    qDebug() << "🔴 main() 종료, 코드:" << result;
    return result;
}
