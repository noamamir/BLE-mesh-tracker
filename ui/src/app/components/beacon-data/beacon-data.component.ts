import {Component, Input, OnChanges, OnDestroy, OnInit, SimpleChanges} from '@angular/core';
import {Beacon} from "../../models/beacon";
import {Observable} from "rxjs";
import {TimeElapsedService} from "../../services/timeElapsed/time-elapsed.service";
import {GridStateService} from "../../services/grid-state/grid-state.service";

@Component({
  selector: 'app-beacon-data',
  templateUrl: './beacon-data.component.html',
  styleUrls: ['./beacon-data.component.css']
})
export class BeaconDataComponent implements OnInit{
  @Input() beaconData: Beacon
  @Input() highlightBeacon: boolean;
  time$: Observable<number>;
  constructor(private timeElapsedService: TimeElapsedService,
              public gridState: GridStateService) {}

  ngOnInit(): void {
    this.time$ = this.timeElapsedService.time()
  }
}
