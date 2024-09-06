import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;

public class ElectionSimulator extends JFrame {
    private static final String[] PARTIES = {"DKP", "SDAP", "DVP", "Z", "AP", "KZP", "KPD"};
    private static final String[] BLOCS = {"Working Class", "New Bourgeoisie", "Old Bourgeoisie", "Rural", "Upper Class"};
    private static final int[] BLOC_PERCENTAGES = {40, 17, 18, 20, 5};

    private static final Map<String, double[]> BASE_SUPPORT = new HashMap<>();
    static {
        BASE_SUPPORT.put("Working Class", new double[]{0.6, 0.3, 0.05, 0.05, 0, 0, 0});
        BASE_SUPPORT.put("New Bourgeoisie", new double[]{0.05, 0.1, 0.5, 0.1, 0.1, 0.05, 0.1});
        BASE_SUPPORT.put("Old Bourgeoisie", new double[]{0, 0.05, 0.2, 0.25, 0.25, 0.15, 0.1});
        BASE_SUPPORT.put("Rural", new double[]{0.1, 0.1, 0.15, 0.2, 0.2, 0.25, 0});
        BASE_SUPPORT.put("Upper Class", new double[]{0, 0.05, 0.25, 0.2, 0.2, 0.15, 0.15});
    }

    private JTextArea resultArea;
    private JButton simulateButton;

    public ElectionSimulator() {
        setTitle("Election Simulator");
        setSize(600, 400);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        resultArea = new JTextArea();
        JScrollPane scrollPane = new JScrollPane(resultArea);
        add(scrollPane, "Center");

        simulateButton = new JButton("Simulate Election");
        simulateButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                simulateElection();
            }
        });
        add(simulateButton, "South");
    }

    private void simulateElection() {
        Random rand = new Random();
        double[] totalVotes = new double[PARTIES.length];

        for (int i = 0; i < BLOCS.length; i++) {
            double[] baseSupport = BASE_SUPPORT.get(BLOCS[i]);
            double blocPercentage = BLOC_PERCENTAGES[i] / 100.0;
            double blocVotes = 0;

            for (int j = 0; j < PARTIES.length; j++) {
                double variation = 0.75 + (rand.nextDouble() * 0.5);
                double votes = baseSupport[j] * variation;
                totalVotes[j] += votes * blocPercentage;
                blocVotes += votes * blocPercentage;
            }

            // Normalize votes for the bloc to 100%
            for (int j = 0; j < PARTIES.length; j++) {
                totalVotes[j] = (totalVotes[j] / blocVotes) * blocPercentage;
            }
        }

        StringBuilder results = new StringBuilder("Election Results:\n");
        double totalPercentage = 0;
        for (int i = 0; i < PARTIES.length; i++) {
            results.append(PARTIES[i]).append(": ").append(String.format("%.2f", totalVotes[i] * 100)).append("%\n");
            totalPercentage += totalVotes[i];
        }
        results.append("\nTotal Percentage: ").append(String.format("%.2f", totalPercentage * 100)).append("%\n");

        // Determine possible coalitions
        results.append("\nPossible Coalitions:\n");
        checkCoalition(results, totalVotes, "Left Front", new int[]{0, 1});
        checkCoalition(results, totalVotes, "Popular Front", new int[]{0, 1, 3});
        checkCoalition(results, totalVotes, "Grand Coalition", new int[]{0, 1, 2, 3, 4, 5, 6});

        resultArea.setText(results.toString());
    }

    private void checkCoalition(StringBuilder results, double[] totalVotes, String name, int[] coalitionIndices) {
        double total = 0;
        for (int index : coalitionIndices) {
            total += totalVotes[index];
        }
        if (total > 0.5) {
            results.append(name).append(": ").append(String.format("%.2f", total * 100)).append("%\n");
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                new ElectionSimulator().setVisible(true);
            }
        });
    }
}
