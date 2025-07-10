import sys
from PyQt6.QtCore import Qt, QPoint, QRect
from PyQt6.QtGui import QColor, QPainter
from PyQt6.QtWidgets import QApplication, QWidget, QColorDialog

class ColorSquare(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.WindowStaysOnTopHint)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setMinimumSize(100, 100)
        self.resize(200, 200)

        self.color = QColor("blue")
        self.drag_position = QPoint()
        self.resizing = False
        self.resize_direction = None
        self.margin = 8  # 判定边缘大小

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        painter.setBrush(self.color)
        painter.setPen(Qt.PenStyle.NoPen)
        painter.drawRect(self.rect())

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self.resize_direction = self._getResizeDirection(event.pos())
            if self.resize_direction != "none":
                self.resizing = True
                self.start_pos = event.globalPosition().toPoint()
                self.start_geom = self.geometry()
            else:
                self.drag_position = event.globalPosition().toPoint() - self.frameGeometry().topLeft()

        elif event.button() == Qt.MouseButton.RightButton:
            color = QColorDialog.getColor(self.color, self, "选择颜色")
            if color.isValid():
                self.color = color
                self.update()

    def mouseMoveEvent(self, event):
        pos = event.pos()
        global_pos = event.globalPosition().toPoint()

        if event.buttons() & Qt.MouseButton.LeftButton:
            if self.resizing:
                dx = global_pos.x() - self.start_pos.x()
                dy = global_pos.y() - self.start_pos.y()
                rect = QRect(self.start_geom)

                if "left" in self.resize_direction:
                    rect.setLeft(rect.left() + dx)
                if "right" in self.resize_direction:
                    rect.setRight(rect.right() + dx)
                if "top" in self.resize_direction:
                    rect.setTop(rect.top() + dy)
                if "bottom" in self.resize_direction:
                    rect.setBottom(rect.bottom() + dy)

                # 最小尺寸限制
                if rect.width() < self.minimumWidth():
                    rect.setWidth(self.minimumWidth())
                if rect.height() < self.minimumHeight():
                    rect.setHeight(self.minimumHeight())

                self.setGeometry(rect)

            else:
                self.move(global_pos - self.drag_position)

        else:
            # 改变光标样式
            direction = self._getResizeDirection(pos)
            cursor = {
                "top": Qt.CursorShape.SizeVerCursor,
                "bottom": Qt.CursorShape.SizeVerCursor,
                "left": Qt.CursorShape.SizeHorCursor,
                "right": Qt.CursorShape.SizeHorCursor,
                "top_left": Qt.CursorShape.SizeFDiagCursor,
                "bottom_right": Qt.CursorShape.SizeFDiagCursor,
                "top_right": Qt.CursorShape.SizeBDiagCursor,
                "bottom_left": Qt.CursorShape.SizeBDiagCursor,
            }.get(direction, Qt.CursorShape.ArrowCursor)
            self.setCursor(cursor)

    def mouseReleaseEvent(self, event):
        self.resizing = False
        self.resize_direction = None

    def keyPressEvent(self, event):
        if event.key() == Qt.Key.Key_Tab and event.modifiers() & Qt.KeyboardModifier.ControlModifier:
            flags = self.windowFlags()
            if flags & Qt.WindowType.WindowStaysOnTopHint:
                print("切换为普通窗口")
                flags &= ~Qt.WindowType.WindowStaysOnTopHint
            else:
                print("切换为置顶窗口")
                flags |= Qt.WindowType.WindowStaysOnTopHint
            self.setWindowFlags(flags)
            self.show()

    def _getResizeDirection(self, pos):
        x, y = pos.x(), pos.y()
        w, h = self.width(), self.height()
        m = self.margin

        in_left = x < m
        in_right = x > w - m
        in_top = y < m
        in_bottom = y > h - m

        if in_top and in_left:
            return "top_left"
        elif in_top and in_right:
            return "top_right"
        elif in_bottom and in_left:
            return "bottom_left"
        elif in_bottom and in_right:
            return "bottom_right"
        elif in_top:
            return "top"
        elif in_bottom:
            return "bottom"
        elif in_left:
            return "left"
        elif in_right:
            return "right"
        else:
            return "none"

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = ColorSquare()
    win.show()
    sys.exit(app.exec())