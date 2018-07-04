package cz.crcs.ectester.reader.test;

import cz.crcs.ectester.applet.ECTesterApplet;
import cz.crcs.ectester.applet.EC_Consts;
import cz.crcs.ectester.common.ec.EC_Curve;
import cz.crcs.ectester.common.ec.EC_Key;
import cz.crcs.ectester.common.output.TestWriter;
import cz.crcs.ectester.common.test.CompoundTest;
import cz.crcs.ectester.common.test.Test;
import cz.crcs.ectester.common.util.CardUtil;
import cz.crcs.ectester.data.EC_Store;
import cz.crcs.ectester.reader.CardMngr;
import cz.crcs.ectester.reader.ECTesterReader;
import cz.crcs.ectester.reader.command.Command;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import static cz.crcs.ectester.common.test.Result.ExpectedValue;

/**
 * @author Jan Jancar johny@neuromancer.sk
 */
public class CardCompositeSuite extends CardTestSuite {

    public CardCompositeSuite(TestWriter writer, ECTesterReader.Config cfg, CardMngr cardManager) {
        super(writer, cfg, cardManager, "composite", "The composite suite runs ECDH over curves with composite order. This should generally fail, as using such a curve is unsafe.");
    }

    @Override
    protected void runTests() throws Exception {
        /* Do the default run with the public keys set to provided smallorder keys
         * over composite order curves. Essentially small subgroup attacks.
         * These should fail, the curves aren't safe so that if the computation with
         * a small order public key succeeds the private key modulo the public key order
         * is revealed.
         */
        Map<String, EC_Key> keys = EC_Store.getInstance().getObjects(EC_Key.class, "composite");
        List<Map.Entry<EC_Curve, List<EC_Key>>> mappedKeys = EC_Store.mapKeyToCurve(keys.values());
        for (Map.Entry<EC_Curve, List<EC_Key>> curveKeys : mappedKeys) {
            EC_Curve curve = curveKeys.getKey();
            List<Test> tests = new LinkedList<>();
            Test allocate = runTest(CommandTest.expect(new Command.Allocate(this.card, ECTesterApplet.KEYPAIR_LOCAL, curve.getBits(), curve.getField()), ExpectedValue.SUCCESS));
            if (!allocate.ok()) {
                doTest(CompoundTest.all(ExpectedValue.SUCCESS, "No support for " + curve.getBits() + "b " + CardUtil.getKeyTypeString(curve.getField()) + ".", allocate));
                continue;
            }
            tests.add(allocate);
            for (EC_Key key : curveKeys.getValue()) {
                Test set = CommandTest.expect(new Command.Set(this.card, ECTesterApplet.KEYPAIR_LOCAL, EC_Consts.CURVE_external, curve.getParams(), curve.flatten()), ExpectedValue.ANY);
                Test generate = CommandTest.expect(new Command.Generate(this.card, ECTesterApplet.KEYPAIR_LOCAL), ExpectedValue.ANY);
                Command ecdhCommand = new Command.ECDH_direct(this.card, ECTesterApplet.KEYPAIR_LOCAL, ECTesterApplet.EXPORT_FALSE, EC_Consts.TRANSFORMATION_NONE, EC_Consts.KeyAgreement_ALG_EC_SVDP_DH, key.flatten());
                Test ecdh = CommandTest.expect(ecdhCommand, ExpectedValue.FAILURE, "Card correctly rejected to do ECDH over a composite order curve.", "Card incorrectly does ECDH over a composite order curve, leaks bits of private key.");

                tests.add(CompoundTest.greedyAllTry(ExpectedValue.SUCCESS, "Composite test of " + curve.getId() + ", " + key.getDesc(), set, generate, ecdh));
            }
            doTest(CompoundTest.all(ExpectedValue.SUCCESS, "Composite test of " + curve.getId() + ".", tests.toArray(new Test[0])));
            new Command.Cleanup(this.card).send();
        }

        /* Also test having a G of small order, so small R.
         */
        Map<String, EC_Curve> results = EC_Store.getInstance().getObjects(EC_Curve.class, "composite");
        List<Map.Entry<String, List<EC_Curve>>> groupList = EC_Store.mapToPrefix(results.values());
        List<EC_Curve> smallRCurves = groupList.stream().filter((e) -> e.getKey().equals("small")).findFirst().get().getValue();
        testGroup(smallRCurves, "Small generator order", ExpectedValue.FAILURE, "Card correctly rejected to do ECDH over a small order generator.", "Card incorrectly does ECDH over a small order generator.");

        /* Also test having a G of large but composite order, R = p * q,
         */
        List<EC_Curve> pqCurves = groupList.stream().filter((e) -> e.getKey().equals("pq")).findFirst().get().getValue();
        testGroup(pqCurves, null, ExpectedValue.ANY, "", "");

        /* Also test rg0 curves.
         */
        List<EC_Curve> rg0Curves = groupList.stream().filter((e) -> e.getKey().equals("rg0")).findFirst().get().getValue();
        testGroup(rg0Curves, null, ExpectedValue.ANY, "", "");
    }

    private void testGroup(List<EC_Curve> curves, String testName, ExpectedValue dhValue, String ok, String nok) throws Exception {
        for (EC_Curve curve : curves) {
            Test allocate = CommandTest.expect(new Command.Allocate(this.card, ECTesterApplet.KEYPAIR_BOTH, curve.getBits(), curve.getField()), ExpectedValue.SUCCESS);
            Test set = CommandTest.expect(new Command.Set(this.card, ECTesterApplet.KEYPAIR_BOTH, EC_Consts.CURVE_external, curve.getParams(), curve.flatten()), ExpectedValue.ANY);
            Test generate = CommandTest.expect(new Command.Generate(this.card, ECTesterApplet.KEYPAIR_BOTH), ExpectedValue.ANY);
            Test ecdh = CommandTest.expect(new Command.ECDH(this.card, ECTesterApplet.KEYPAIR_LOCAL, ECTesterApplet.KEYPAIR_REMOTE, ECTesterApplet.EXPORT_FALSE, EC_Consts.TRANSFORMATION_NONE, EC_Consts.KeyAgreement_ALG_EC_SVDP_DH), dhValue, ok, nok);

            String description;
            if (testName == null) {
                description = curve.getDesc() + " test of " + curve.getId() + ".";
            } else {
                description = testName + " test of " + curve.getId() + ".";
            }
            doTest(CompoundTest.greedyAllTry(ExpectedValue.SUCCESS, description, allocate, set, generate, ecdh));
            new Command.Cleanup(this.card).send();
        }

    }
}