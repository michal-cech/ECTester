package cz.crcs.ectester.reader.output;

import cz.crcs.ectester.common.output.TestWriter;
import cz.crcs.ectester.common.test.CompoundTest;
import cz.crcs.ectester.common.test.Test;
import cz.crcs.ectester.common.test.TestSuite;
import cz.crcs.ectester.reader.test.CommandTest;
import cz.crcs.ectester.reader.test.CardTestSuite;

import java.io.PrintStream;

/**
 * @author Jan Jancar johny@neuromancer.sk
 */
public class TextTestWriter implements TestWriter {
    private PrintStream output;
    private ResponseWriter respWriter;

    public static int BASE_WIDTH = 76;

    public TextTestWriter(PrintStream output) {
        this.output = output;
        this.respWriter = new ResponseWriter(output);
    }

    @Override
    public void begin(TestSuite suite) {
        output.println("=== Running test suite: " + suite.getName() + " ===");
        output.println("=== " + suite.getDescription());
    }

    private String testString(Test t, int offset) {
        if (!t.hasRun()) {
            return null;
        }

        StringBuilder out = new StringBuilder();
        if (t instanceof CommandTest) {
            CommandTest test = (CommandTest) t;
            out.append(test.ok() ? "OK  " : "NOK ");
            out.append("━ ");
            int width = BASE_WIDTH - (offset + out.length());
            String widthSpec = "%-" + String.valueOf(width) + "s";
            out.append(String.format(widthSpec, t.getDescription()));
            out.append(" ┃ ");
            out.append(String.format("%-9s", test.getResultValue().name()));
            out.append(" ┃ ");
            out.append(respWriter.responseSuffix(test.getResponse()));
        } else {
            CompoundTest test = (CompoundTest) t;
            out.append(test.ok() ? "OK  " : "NOK ");
            out.append("┳ ");
            int width = BASE_WIDTH - (offset + out.length());
            String widthSpec = "%-" + String.valueOf(width) + "s";
            out.append(String.format(widthSpec, t.getDescription()));
            out.append(" ┃ ");
            out.append(String.format("%-9s", test.getResultValue().name()));
            out.append(" ┃ ");
            out.append(test.getResultCause());
            out.append(System.lineSeparator());
            Test[] tests = test.getTests();
            for (int i = 0; i < tests.length; ++i) {
                if (i == tests.length - 1) {
                    out.append("    ┗ ");
                } else {
                    out.append("    ┣ ");
                }
                out.append(testString(tests[i], offset + 6));
                if (i != tests.length - 1) {
                    out.append(System.lineSeparator());
                }
            }
        }

        return out.toString();
    }

    @Override
    public void outputTest(Test t) {
        if (!t.hasRun())
            return;
        output.println(testString(t, 0));
        output.flush();
    }

    @Override
    public void end() {
    }
}
