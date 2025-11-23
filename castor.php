#!/usr/bin/env -S castor --castor-file
<?php

use Castor\Attribute\AsTask;

use function Castor\capture;
use function Castor\io;
use function Castor\run;

#[AsTask(description: 'Start the latest stopped docker project')]
function start(): void
{
    $projects = get_docker_projects();

    if (count($projects) === 0) {
        io()->warning('No docker projects found.');

        return;
    }

    $latestProject = $projects[0];

    io()->note("Starting docker project {$latestProject['project']}");

    docker_compose($latestProject, 'start');
}

#[AsTask(description: 'Start the previous docker project')]
function previous(): void
{
    stop();

    $projects = get_docker_projects();

    if (count($projects) < 1) {
        io()->warning('No more than 2 docker projects found.');

        return;
    }

    $previousProject = $projects[1];

    io()->note("Starting docker project {$previousProject['project']}");

    docker_compose($previousProject, 'start');
}

#[AsTask(description: 'Stop running docker projects')]
function stop(): void
{
    $projects = get_docker_projects();

    $running = array_filter($projects, fn (array $p) => $p['running']);

    if (count($running) === 0) {
        io()->warning('No running docker projects found.');

        return;
    }

    foreach ($running as $project) {
        io()->note("Stopping docker project {$project['project']}");

        docker_compose($project, 'stop');
    }
}

/**
 * @return list<array{project: string, working_dir: ?string, running: bool, containers: list<array{id: string, project: string, working_dir: ?string, running: bool, finishedAt: ?string}>, lastStoppedAt: ?string}>
 */
function get_docker_projects(): array
{
    $output = capture(['docker', 'ps', '-a', '--format', '{{.ID}}']);
    $containerIds = array_filter(array_map('trim', explode("\n", $output)));

    $containers = [];

    // Inspect each container to get detailed info
    foreach ($containerIds as $id) {
        $json = capture(['docker', 'inspect', $id]);
        $data = json_decode($json, true)[0] ?? null;

        if (!$data) {
            continue;
        }

        $labels = $data['Config']['Labels'] ?? [];

        // Only keep containers managed by Docker Compose
        if (!isset($labels['com.docker.compose.project'])) {
            continue;
        }

        // Exclude builder containers
        if (str_ends_with($data['Config']['Image'], 'builder')) {
            continue;
        }

        $containers[] = [
            'id' => $data['Id'],
            'project' => $labels['com.docker.compose.project'],
            'working_dir' => $labels['com.docker.compose.project.working_dir'] ?? null,
            'running' => $data['State']['Running'] ?? false,
            'finishedAt' => $data['State']['FinishedAt'] ?? null,
        ];
    }

    // Group containers by docker compose project
    $projects = [];

    foreach ($containers as $c) {
        $name = $c['project'];

        if (!($projects[$name] ?? false)) {
            $projects[$name] = [
                'project' => $name,
                'workingDir' => $c['working_dir'],
                'running' => false,
                'lasStoppedAt' => null,
                'containers' => [],
            ];
        }

        $projects[$name]['running'] = $projects[$name]['running'] ?: $c['running'];
        $projects[$name]['containers'][] = $c;
    }

    $projects = array_values($projects);

    // Detect last stopped containers per project
    foreach ($projects as &$project) {
        $cs = $project['containers'];

        // Filter to avoid null dates
        $stopped = array_filter($cs, fn ($c) => $c['finishedAt'] && $c['finishedAt'] !== '0001-01-01T00:00:00Z');

        // Sort to get the latest
        usort($stopped, fn ($a, $b) => $a['finishedAt'] <=> $b['finishedAt']);

        $c = end($stopped) ?: null;

        if ($c) {
            $project['lasStoppedAt'] = $c['finishedAt'];
        }
    }

    // Sort projects by last stopped date descending
    usort($projects, function ($a, $b) {
        $aDate = $a['lasStoppedAt'] ?? null;
        $bDate = $b['lasStoppedAt'] ?? null;

        return $bDate <=> $aDate;
    });

    return $projects;
}

function docker_compose(array $project, string $command): void
{
    run(['docker', 'compose', '-p', $project['project'], $command]);
}
