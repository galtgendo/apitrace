#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "trace_api.hpp"
#include "apitrace.h"

#include <QMainWindow>
#include <QProcess>
#include <QList>
#include <QImage>

class ApiTrace;
class ApiTraceCall;
class ApiTraceEvent;
class ApiTraceFilter;
class ApiTraceFrame;
class ApiTraceModel;
class ApiTraceState;
class ArgumentsEditor;
class JumpWidget;
class QModelIndex;
class QProgressBar;
class QTreeWidgetItem;
class QUrl;
struct ApiTraceError;
class Retracer;
class SearchWidget;
class ShadersSourceWidget;
class TraceProcess;
class TrimProcess;
class ProfileDialog;
class VertexDataInterpreter;

namespace trace { struct Profile; }

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

public slots:
    void loadTrace(const QString &fileName, int callNum = -1);

private slots:
    void callItemSelected(const QModelIndex &index);
    void callItemActivated(const QModelIndex &index);
    void createTrace();
    void openTrace();
    void replayStart();
    void replayProfile();
    void replayStop();
    void replayFinished(const QString &message);
    void replayStateFound(ApiTraceState *state);
    void replayProfileFound(trace::Profile *state);
    void replayThumbnailsFound(const QList<QImage> &thumbnails);
    void replayError(const QString &msg);
    void startedLoadingTrace();
    void loadProgess(int percent);
    void finishedLoadingTrace();
    void lookupState();
    void showThumbnails();
    void trim();
    void showSettings();
    void openHelp(const QUrl &url);
    void showSurfacesMenu(const QPoint &pos);
    void showSelectedSurface();
    void saveSelectedSurface();
    void slotGoTo();
    void slotJumpTo(int callNum);
    void createdTrace(const QString &path);
    void traceError(const QString &msg);
    void createdTrim(const QString &path);
    void trimError(const QString &msg);
    void slotSearch();
    void slotSearchNext(const QString &str, Qt::CaseSensitivity sensitivity);
    void slotSearchPrev(const QString &str, Qt::CaseSensitivity sensitivity);
    void fillState(bool nonDefaults);
    void customContextMenuRequested(QPoint pos);
    void editCall();
    void slotStartedSaving();
    void slotSaved();
    void slotGoFrameStart();
    void slotGoFrameEnd();
    void slotTraceChanged(ApiTraceEvent *event);
    void slotRetraceErrors(const QList<ApiTraceError> &errors);
    void slotErrorSelected(QTreeWidgetItem *current);
    void slotSearchResult(const ApiTrace::SearchRequest &request,
                          ApiTrace::SearchResult result,
                          ApiTraceCall *call);
    void slotFoundFrameStart(ApiTraceFrame *frame);
    void slotFoundFrameEnd(ApiTraceFrame *frame);
    void slotJumpToResult(ApiTraceCall *call);

private:
    void initObjects();
    void initConnections();
    void newTraceFile(const QString &fileName);
    void replayTrace(bool dumpState, bool dumpThumbnails);
    void trimEvent();
    void fillStateForFrame();

    /* there's a difference between selected frame/call and
     * current call/frame. the former implies actual selection
     * the latter might be just a highlight, e.g. during searching
     */
    ApiTraceFrame *selectedFrame() const;
    ApiTraceCall *selectedCall() const;
    ApiTraceFrame *currentFrame() const;
    ApiTraceCall *currentCall() const;

protected:
    virtual void closeEvent(QCloseEvent * event);

private:
    Ui_MainWindow m_ui;
    ShadersSourceWidget *m_sourcesWidget;

    trace::API m_api;

    ApiTrace *m_trace;
    ApiTraceModel *m_model;
    ApiTraceFilter *m_proxyModel;
    int m_initalCallNum;

    QProgressBar *m_progressBar;

    ApiTraceEvent *m_selectedEvent;

    ApiTraceEvent *m_stateEvent;

    ApiTraceEvent *m_trimEvent;

    Retracer *m_retracer;

    VertexDataInterpreter *m_vdataInterpreter;

    JumpWidget *m_jumpWidget;
    SearchWidget *m_searchWidget;

    TraceProcess *m_traceProcess;

    TrimProcess *m_trimProcess;

    ArgumentsEditor *m_argsEditor;

    ApiTraceEvent *m_nonDefaultsLookupEvent;

    ProfileDialog* m_profileDialog;
};


#endif
