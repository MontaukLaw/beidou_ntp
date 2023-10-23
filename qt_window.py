import sys
from PyQt5.QtWidgets import QApplication, QWidget, QLabel
from PyQt5.QtCore import QTimer, QDateTime

class DateDisplayApp(QWidget):
    def __init__(self):
        super().__init__()

        self.initUI()
        self.updateDate()

    def initUI(self):
        self.setWindowTitle("系统日期显示")
        self.setGeometry(100, 100, 300, 100)

        self.date_label = QLabel(self)
        self.date_label.setGeometry(10, 10, 280, 80)
        self.date_label.setStyleSheet("font-size: 20px")

    def updateDate(self):
        current_date = QDateTime.currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
        self.date_label.setText(current_date)

        timer = QTimer(self)
        timer.timeout.connect(self.updateDate)
        timer.start(1000)  # 1秒钟更新一次日期

def main():
    app = QApplication(sys.argv)
    ex = DateDisplayApp()
    ex.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()