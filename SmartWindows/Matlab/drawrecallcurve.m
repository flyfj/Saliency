
% recall vs. #proposal

%pr_vals = load('all_pr.txt');
figure
plot(1:size(pr_vals,1), pr_vals(:,1), 'r-')
grid on
title('Recall vs. #.proposal on RGBD Saliency')
xlabel('#.Proposal')
ylabel('Recall (0.6)')
hold on
