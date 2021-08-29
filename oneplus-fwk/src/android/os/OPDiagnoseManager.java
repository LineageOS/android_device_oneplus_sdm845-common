package android.os;

public final class OPDiagnoseManager {

    private static String LOG_TAG = OPDiagnoseManager.class.getSimpleName();

    public boolean addIssueCount(int type, int count) {
        return true;
    }

    public boolean setIssueNumber(int type, int count) {
        return true;
    }

    public boolean writeDiagData(int type, String issueDesc) {
        return true;
    }

    public boolean setDiagData(int type, String issueDesc, int count) {
        return true;
    }

    public boolean saveDiagLog(int type) {
        return true;
    }

    public boolean saveQxdmLog(int type, String mask_type) {
        return true;
    }

    public boolean readDiagData(int type) {
        return true;
    }
}
