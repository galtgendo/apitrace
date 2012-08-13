#include "profiletablemodel.h"

typedef trace::Profile::Call Call;
typedef trace::Profile::Frame Frame;

enum {
    COLUMN_PROGRAM,
    COLUMN_USAGES,
    COLUMN_GPU_TIME,
    COLUMN_CPU_TIME,
    COLUMN_PIXELS_DRAWN,
    COLUMN_GPU_AVERAGE,
    COLUMN_CPU_AVERAGE,
    COLUMN_PIXELS_AVERAGE,
    MAX_COLUMN
};

static QString columnNames[] = {
    QString("Program"),
    QString("Calls"),
    QString("Total GPU Time"),
    QString("Total CPU Time"),
    QString("Total Pixels Drawn"),
    QString("Avg GPU Time"),
    QString("Avg CPU Time"),
    QString("Avg Pixels Drawn")
};

ProfileTableModel::ProfileTableModel(QObject *parent)
    : QAbstractTableModel(parent),
      m_profile(0),
      m_sortColumn(COLUMN_GPU_TIME),
      m_sortOrder(Qt::DescendingOrder)
{
}


void ProfileTableModel::setProfile(trace::Profile* profile)
{
    m_profile = profile;
    m_timeMin = m_profile->frames.front().gpuStart;
    m_timeMax = m_profile->frames.back().gpuStart + m_profile->frames.back().gpuDuration;
    updateModel();
}


void ProfileTableModel::setTimeSelection(int64_t start, int64_t end)
{
    m_timeMin = start;
    m_timeMax = end;
    updateModel();
    sort(m_sortColumn, m_sortOrder);
}


/**
 * Creates the row data from trace profile
 */
void ProfileTableModel::updateModel()
{
    if (m_timeMin == m_timeMax) {
        m_timeMin = m_profile->frames.front().gpuStart;
        m_timeMax = m_profile->frames.back().gpuStart + m_profile->frames.back().gpuDuration;
    }

    for (QList<ProfileTableRow>::iterator itr = m_rowData.begin(); itr != m_rowData.end(); ++itr) {
        ProfileTableRow& row = *itr;

        row.uses = 0;
        row.pixels = 0;
        row.gpuTime = 0;
        row.cpuTime = 0;
        row.longestCpu = NULL;
        row.longestGpu = NULL;
        row.longestPixel = NULL;
    }

    for (Frame::const_iterator itr = m_profile->frames.begin(); itr != m_profile->frames.end(); ++itr) {
        const Frame& frame = *itr;

        if (frame.gpuStart > m_timeMax) {
            break;
        }

        if ((frame.gpuStart + frame.gpuDuration) < m_timeMin) {
            continue;
        }

        for (Call::const_iterator jtr = frame.calls.begin(); jtr != frame.calls.end(); ++jtr) {
            const Call& call = *jtr;

            if (call.gpuStart > m_timeMax) {
                break;
            }

            if ((call.gpuStart + call.gpuDuration) < m_timeMin) {
                continue;
            }

            ProfileTableRow* row = getRow(call.program);
            if (!row) {
                m_rowData.append(ProfileTableRow());
                row = &m_rowData.back();
            }

            row->uses++;
            row->program  = call.program;
            row->gpuTime += call.gpuDuration;
            row->cpuTime += call.cpuDuration;
            row->pixels  += call.pixels;

            if (!row->longestGpu || row->longestGpu->gpuDuration < call.gpuDuration) {
                row->longestGpu = &call;
            }

            if (!row->longestCpu || row->longestCpu->cpuDuration < call.cpuDuration) {
                row->longestCpu = &call;
            }

            if (!row->longestPixel || row->longestPixel->pixels < call.pixels) {
                row->longestPixel = &call;
            }
        }
    }
}


/**
 * Get the appropriate call associated with an item in the table
 */
const Call* ProfileTableModel::getJumpCall(const QModelIndex & index) const {
    const ProfileTableRow& row = m_rowData[index.row()];

    switch(index.column()) {
    case COLUMN_GPU_TIME:
    case COLUMN_GPU_AVERAGE:
        return row.longestGpu;
    case COLUMN_CPU_TIME:
    case COLUMN_CPU_AVERAGE:
        return row.longestCpu;
    case COLUMN_PIXELS_DRAWN:
    case COLUMN_PIXELS_AVERAGE:
        return row.longestPixel;
    }

    return NULL;
}


ProfileTableRow* ProfileTableModel::getRow(unsigned program) {
    for (QList<ProfileTableRow>::iterator itr = m_rowData.begin(); itr != m_rowData.end(); ++itr) {
        if (itr->program == program)
            return &*itr;
    }

    return NULL;
}


int ProfileTableModel::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid()) {
        return m_rowData.size();
    } else {
        return 0;
    }
}


int ProfileTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return MAX_COLUMN;
}


QVariant ProfileTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        const ProfileTableRow& row = m_rowData[index.row()];

        switch(index.column()) {
        case COLUMN_PROGRAM:
            return row.program;
        case COLUMN_USAGES:
            return row.uses;
        case COLUMN_GPU_TIME:
            return row.gpuTime;
        case COLUMN_CPU_TIME:
            return row.cpuTime;
        case COLUMN_PIXELS_DRAWN:
            return row.pixels;
        case COLUMN_GPU_AVERAGE:
            return (row.uses <= 0) ? 0 : (row.gpuTime / row.uses);
        case COLUMN_CPU_AVERAGE:
            return (row.uses <= 0) ? 0 : (row.cpuTime / row.uses);
        case COLUMN_PIXELS_AVERAGE:
            return (row.uses <= 0) ? 0 : (row.pixels / row.uses);
        }
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }

    return QVariant();
}


QVariant ProfileTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section >= 0 && section < MAX_COLUMN) {
            return columnNames[section];
        }
    }

    return QVariant();
}


class ProgramSorter {
public:
    ProgramSorter(int column, Qt::SortOrder order)
        : mSortColumn(column),
          mSortOrder(order)
    {
    }

    bool operator()(const ProfileTableRow &p1, const ProfileTableRow &p2) const
    {
        bool result = true;

        switch(mSortColumn) {
        case COLUMN_PROGRAM:
            result = p1.program < p2.program;
            break;
        case COLUMN_USAGES:
            result = p1.uses < p2.uses;
            break;
        case COLUMN_GPU_TIME:
            result = p1.gpuTime < p2.gpuTime;
            break;
        case COLUMN_CPU_TIME:
            result = p1.cpuTime < p2.cpuTime;
            break;
        case COLUMN_PIXELS_DRAWN:
            result = p1.pixels < p2.pixels;
            break;
        case COLUMN_GPU_AVERAGE:
            result = ((p1.uses <= 0) ? 0 : (p1.gpuTime / p1.uses)) < ((p2.uses <= 0) ? 0 : (p2.gpuTime / p2.uses));
            break;
        case COLUMN_CPU_AVERAGE:
            result = ((p1.uses <= 0) ? 0 : (p1.cpuTime / p1.uses)) < ((p2.uses <= 0) ? 0 : (p2.cpuTime / p2.uses));
            break;
        case COLUMN_PIXELS_AVERAGE:
            result = ((p1.uses <= 0) ? 0 : (p1.pixels / p1.uses)) < ((p2.uses <= 0) ? 0 : (p2.pixels / p2.uses));
            break;
        }

        if (mSortOrder == Qt::DescendingOrder) {
            return !result;
        } else {
            return result;
        }
    }

private:
    int mSortColumn;
    Qt::SortOrder mSortOrder;
};


void ProfileTableModel::sort(int column, Qt::SortOrder order) {
    m_sortColumn = column;
    m_sortOrder = order;
    qSort(m_rowData.begin(), m_rowData.end(), ProgramSorter(column, order));
    emit dataChanged(createIndex(0, 0), createIndex(m_rowData.size(), MAX_COLUMN));
}


#include "profiletablemodel.moc"